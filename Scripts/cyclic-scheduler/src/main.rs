fn main() {
    let input = std::env::args().nth(1).unwrap_or("input.txt".to_owned());
    let input = std::fs::read_to_string(input).unwrap();

    let mut smallest = 1.;
    let raw_tasks = input.lines().map(|line| {
        let mut split = line.split(',');
        let name = split.next().expect("Expected task name");
        let computing_time = split.next().and_then(|s| s.parse::<f64>().ok()).expect("Expected computing time");
        let period = split.next().and_then(|s| s.parse::<f64>().ok()).expect("Expected period");

        let fract = computing_time.fract();
        if fract > 0. && fract < smallest {
            smallest = fract;
        }
        let fract = period.fract();
        if fract > 0. && fract < smallest {
            smallest = fract;
        }
        (name, computing_time, period)
    }).collect::<Vec<_>>();

    let base= if smallest < 1. {
        10u64.pow(smallest.log10().abs() as u32)
    } else {
        1
    };

    // Normalize numbers to integers
    let tasks: &mut [Task] = &mut raw_tasks.into_iter().map(|(name, computing_time, period)| {
        Task {
            name: name.to_owned(),
            computing_time: (computing_time * base as f64).round() as u64,
            period: (period * base as f64).round() as u64,
        }
    }).collect::<Vec<_>>();

    dbg!(&tasks);
    dbg!(base);
    dbg!(utilization(tasks));
    let hyperperiod = dbg!(hyperperiod(tasks));
    let secondary_periods = dbg!(secondary_periods(tasks, hyperperiod));
}

fn utilization(tasks: &[Task]) -> f64 {
    tasks.iter().map(|t| t.computing_time as f64 / t.period as f64).sum()
}

fn hyperperiod(tasks: &[Task]) -> u64 {
    tasks.iter().map(|t| t.period).fold(1, num::integer::lcm)
}

fn secondary_periods(tasks: &[Task], hyperperiod: u64) -> Vec<u64> {
    let max_ci = tasks.iter().map(|t| t.computing_time).max().unwrap();
    let min_di = tasks.iter().map(|t| t.period).min().unwrap();
    let mut frames = Vec::new();
    for period in max_ci..=min_di {
        for k in 1.. {
            match (k * period).cmp(&hyperperiod) {
                std::cmp::Ordering::Less => continue,
                std::cmp::Ordering::Equal => {
                    println!("Found secondary period for k = {k}; period = {period}");
                    frames.push(period);
                    break;
                },
                std::cmp::Ordering::Greater => {
                    break;
                },
            }
        }
    }
    frames
}

#[derive(Debug, Clone)]
struct Task {
    name: String,
    computing_time: u64,
    period: u64,
}