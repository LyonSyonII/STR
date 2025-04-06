use num::Integer;

fn main() {
    let input = std::env::args().nth(1).unwrap_or("input.txt".to_owned());
    let input = std::fs::read_to_string(input).unwrap();
    let mut tasks = parse_tasks(input);
    tasks.sort_by_key(|t| t.deadline);
    println!("Scheduling tasks:");
    for task in &tasks {
        println!("{task:#?}");
    }
    let trivial = tasks.iter().all(|t| t.period == t.deadline);

    let utilization = utilization(&tasks);
    println!("\nUtilization = {utilization:.2}");

    if trivial {
        println!("For all tasks Di = Dt; Checking feasiblity for trivial case:");
        if utilization <= 1. {
            println!(
                "Sufficient condition '{utilization:.2} <= 1' is true; Tasks ARE schedulable!'"
            );
        } else {
            println!(
                "Sufficient condition '{utilization:.2} <= 1' is false; Tasks are NOT schedulable'"
            );
        }
        return;
    }

    let hyperperiod = hyperperiod(&tasks);
    println!("Hyperperiod = {hyperperiod}");
    let lstar = lstar(&tasks, utilization);
    println!("L* = {lstar}");

    if response_time_analysis(&tasks, lstar.min(hyperperiod)) {
        println!("Tasks ARE schedulable!");
    } else {
        println!("Tasks NOT schedulable");
    }
}

#[derive(Debug, Clone)]
struct Task {
    name: String,
    computing_time: u64,
    deadline: u64,
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
            let deadline = split
                .next()
                .and_then(|s| s.trim().parse::<f64>().ok())
                .expect("Expected period");
            let period = split
                .next()
                .and_then(|s| s.trim().parse::<f64>().ok())
                .unwrap_or(deadline);

            let fract = computing_time.fract();
            if fract > 0. && fract < smallest {
                smallest = fract;
            }
            let fract = deadline.fract();
            if fract > 0. && fract < smallest {
                smallest = fract;
            }
            let fract = period.fract();
            if fract > 0. && fract < smallest {
                smallest = fract;
            }
            (name, computing_time, deadline, period)
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
        .map(|(name, computing_time, deadline, period)| Task {
            name: name.to_owned(),
            computing_time: (computing_time * base as f64) as u64,
            deadline: (deadline * base as f64) as u64,
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
    tasks.iter().fold(1, |a, b| a.lcm(&b.period))
}

fn lstar(tasks: &[Task], utilization: f64) -> u64 {
    let sum = tasks
        .iter()
        .map(|t| {
            (t.period as f64 - t.deadline as f64) * (t.computing_time as f64 / t.period as f64)
        })
        .sum::<f64>();
    (sum / (1. - utilization)).ceil() as u64
}

fn response_time_analysis(tasks: &[Task], evaluate_at: u64) -> bool {
    println!("\nResponse Time Analysis from 0 to {evaluate_at}\n");
    for task in tasks {
        println!("{}:", task.name);
        for l in (task.deadline..=evaluate_at).step_by(task.period as usize) {
            let sum = tasks
                .iter()
                .map(|t| ((l + t.period - t.deadline) / t.period) * t.computing_time)
                .sum::<u64>();
            print!("g(0, {l}) = {sum} <= {l}");
            if sum <= l {
                println!(" -> OK");
            } else {
                println!(" -> false, not schedulable");
                return false;
            }
        }
        println!();
    }
    true
}
