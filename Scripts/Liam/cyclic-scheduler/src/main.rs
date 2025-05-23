use num::{Float, Integer};

fn main() {
    let input = std::env::args().nth(1).unwrap_or("input.txt".to_owned());
    let input = std::fs::read_to_string(input).unwrap();

    let tasks = &mut parse_tasks(input);

    dbg!(&tasks);
    let utilization = dbg!(utilization(tasks));
    if utilization > 1. {
        println!("utilization greater than 1, schedule does not exist!");
        std::process::exit(1);
    }
    let hyperperiod = dbg!(hyperperiod(tasks));
    let frame_times = dbg!(frame_times(tasks, hyperperiod));
    if frame_times.is_empty() {
        println!("no secondary periods found, schedule does not exist!");
        std::process::exit(1);
    }

    let possible_schedules = frame_times
        .into_iter()
        .map(|frame_time| schedule(tasks, frame_time, hyperperiod).map(|s| (s, frame_time)));
    for (schedule, frame_time) in possible_schedules.flatten() {
        println!("Schedule(frame_time = {frame_time}):");
        for (frame, frame_tasks) in schedule.iter().enumerate() {
            println!("  [Frame {frame}]");
            let mut start = frame_time * frame as u64;
            for (Task { name, computing_time, period }, activation) in frame_tasks {
                let deadline = activation + period;
                println!("  {name}: {{\n    activation: {activation},\n    deadline: {deadline},\n    start: {start}\n  }}");
                start += computing_time;
            }
            println!();
        }
    }
}

#[derive(Debug, Clone)]
struct Task {
    name: String,
    computing_time: u64,
    period: u64,
}


fn parse_tasks(input: String) -> Vec<Task> {
    let mut smallest = 1.;
    let raw_tasks = input
        .lines()
        .map(|line| {
            let mut split = line.split(',');
            let name = split.next().map(str::trim).expect("Expected task name");
            let computing_time = split
                .next()
                .and_then(|s| s.trim().parse::<f64>().ok())
                .expect("Expected computing time");
            let period = split
                .next()
                .and_then(|s| s.trim().parse::<f64>().ok())
                .expect("Expected period");

            let fract = computing_time.fract();
            if fract > 0. && fract < smallest {
                smallest = fract;
            }
            let fract = period.fract();
            if fract > 0. && fract < smallest {
                smallest = fract;
            }
            (name, computing_time, period)
        })
        .collect::<Vec<_>>();

    let base = if smallest < 1. {
        10u64.pow(smallest.log10().abs().ceil() as u32)
    } else {
        1
    };
    dbg!(base);

    // Normalize numbers to integers
    raw_tasks
        .into_iter()
        .map(|(name, computing_time, period)| Task {
            name: name.to_owned(),
            computing_time: (computing_time * base as f64) as u64,
            period: (period * base as f64) as u64,
        })
        .collect::<Vec<_>>()
}

fn utilization(tasks: &[Task]) -> f64 {
    tasks
        .iter()
        .map(|t| t.computing_time as f64 / t.period as f64)
        .sum()
}

fn hyperperiod(tasks: &[Task]) -> u64 {
    tasks.iter().map(|t| t.period).fold(1, num::integer::lcm)
}

fn frame_times(tasks: &[Task], hyperperiod: u64) -> Vec<u64> {
    let max_ci = tasks.iter().map(|t| t.computing_time).max().unwrap();
    let min_di = tasks.iter().map(|t| t.period).min().unwrap();
    let mut periods = Vec::new();
    for period in max_ci..=min_di {
        for k in 1.. {
            match (k * period).cmp(&hyperperiod) {
                std::cmp::Ordering::Less => continue,
                std::cmp::Ordering::Equal => {
                    println!("Found secondary period for k = {k}; period = {period}");
                    periods.push(period);
                    break;
                }
                std::cmp::Ordering::Greater => {
                    break;
                }
            }
        }
    }

    periods.retain(|&period| {
        println!("\nFor Ts = {period}");
        tasks.iter().all(|task| {
            let task_period = task.period;
            let gcd = period.gcd(&task_period);
            let op = 2 * period - gcd;
            let res = op <= task_period;
            println!("2 * {period} - gcd({period},{task_period}) <= {task_period} = {op} <= {task_period}; {res}");

            res
        })
    });
    println!();

    periods
}

fn schedule(
    tasks: &mut [Task],
    frame_time: u64,
    hyperperiod: u64,
) -> Option<Vec<Vec<(Task, u64)>>> {
    tasks.sort_unstable_by(|a, b| a.period.cmp(&b.period));

    let num_frames = hyperperiod / frame_time;
    let mut available_time = vec![frame_time; num_frames as usize + 1];
    let mut schedule = vec![Vec::<(Task, u64)>::new(); num_frames as usize + 1];
    for task in tasks {
        let num_jobs = hyperperiod / task.period;
        for i in 0..num_jobs {
            let frame = (task.period * i).div_ceil(frame_time);
            let mut scheduled = false;
            let mut c_off = 0;
            while frame + c_off <= num_frames && c_off * frame_time <= task.period {
                let c_fr = (frame + c_off) as usize;
                if available_time[c_fr] >= task.computing_time {
                    schedule[c_fr].push((task.clone(), i * task.period));
                    available_time[c_fr] -= task.computing_time;
                    scheduled = true;
                    break;
                }
                c_off += 1;
            }
            if !scheduled {
                println!("Can't schedule period #{i} of task {}", task.name);
                return None;
            }
        }
    }

    Some(schedule)
}