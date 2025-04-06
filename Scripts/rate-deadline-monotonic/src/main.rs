fn main() {
    let input = std::env::args().nth(1).unwrap_or("input.txt".to_owned());
    let input = std::fs::read_to_string(input).unwrap();
    let mut tasks = parse_tasks(input);
    tasks.sort_by_key(|t| t.deadline);
    println!("Scheduling tasks:");
    for task in &tasks {
        println!("{task:#?}");
    }
    let is_rate_monotonic = tasks.iter().all(|t| t.period == t.deadline);

    let utilization = utilization(&tasks);
    println!("\nutilization = {utilization:.2}");

    if is_rate_monotonic {
        let schedulable_utilization = tasks.len() as f64 * (2f64.powf(1f64/tasks.len() as f64) - 1.);
        println!("n( 2^(1/n) - 1) = {schedulable_utilization:.2}");
        if utilization <= schedulable_utilization {
            println!("Sufficient condition '{utilization:.2} <= {schedulable_utilization:.2}' is true; Tasks ARE schedulable!'");
            return;
        }
        println!("Sufficient condition '{utilization:.2} <= {schedulable_utilization:.2}' is false; Continuing'");

        let hyperbolic = hyperbolic_bound(&tasks);
        println!("\nHyperbolic bound = {hyperbolic:.2}");
        if hyperbolic <= 2. {
            println!("Sufficient condition '{hyperbolic:.2} <= 2' is true; Tasks ARE schedulable!'");
            return;
        }
        println!("Sufficient condition '{hyperbolic:.2} <= 2' is false; Continuing'");
    }

    if response_time_analysis(&tasks) {
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
            let name = split.next().expect("Expected task name");
            let computing_time = split
                .next()
                .and_then(|s| s.parse::<f64>().ok())
                .expect("Expected computing time");
            let deadline = split
                .next()
                .and_then(|s| s.parse::<f64>().ok())
                .expect("Expected period");
            let period = split
                .next()
                .and_then(|s| s.parse::<f64>().ok())
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

fn hyperbolic_bound(tasks: &[Task]) -> f64 {
    tasks.iter()
        .map(|t| (t.computing_time as f64 / t.period as f64) + 1.)
        .fold(1., |a, b| a * b)
}

fn response_time_analysis(tasks: &[Task]) -> bool {
    fn inner(task: &Task, interferences: &[Task], prev: u64) -> bool {
        print!("{}", task.computing_time);
        let new = task.computing_time
            + interferences
                .iter()
                .map(|&Task { computing_time, period, .. }| { 
                    print!(" + ceil({prev}/{period})*{computing_time}");
                    prev.div_ceil(period) * computing_time 
                })
                .sum::<u64>();
        print!(" = {new}");
        if new == prev {
            println!(" == {prev} -> Done!");
            true
        } else if new > task.deadline {
            println!(" > {} -> Not schedulable!", task.deadline);
            false
        } else {
            println!(" <= {} -> Continuing", task.deadline);
            inner(task, interferences, new)
        }
    }

    for (i, task) in tasks.iter().enumerate() {
        println!("\nChecking {}:", task.name);
        if !inner(task, &tasks[..i], 0) {
            return false;
        }
    }
    true

}
