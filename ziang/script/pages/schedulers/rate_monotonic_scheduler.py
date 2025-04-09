import streamlit as st
import altair as alt
from script.pages.utils.navigation import navigate_to
from script.schedulers.rate_monotonic import RateMonotonicScheduler
from script.pages.utils.scheduler import show_basic_scheduler_info
from script.pages.utils.chart import create_chart
from script.utils.task import ResponseTimeAnalysis


def run():
    st.title("Rate Monotonic Scheduler")

    st.button("Back", on_click=navigate_to, args=("home",))

    tasks = st.session_state.get("tasks", [])
    if not tasks:
        st.warning("Please add tasks first.")
        return

    scheduler = RateMonotonicScheduler(tasks)
    show_basic_scheduler_info(scheduler)

    st.write("### Rate Monotonic scheduler Info")

    is_schedulable = scheduler.is_schedulable()
    if is_schedulable:
        st.success("The tasks are schedulable.")
    else:
        st.error("The tasks are not schedulable.")

    condition1 = scheduler.condition1()
    icon_condition1 = "✅" if condition1 else "❌"
    with st.expander("Condition 1", expanded=False, icon=icon_condition1):
        st.write("The total utilization must be less than or equal to $n(\\sqrt[n]2 - 1)$")
        st.write(f"- $n$ = {len(scheduler.tasks)} (number of tasks)")
        st.write(f"- Utilization bound: ${scheduler.utilization_bound:.5f}$")

        condition1 = scheduler.condition1()
        message = f"Total utilization is ${scheduler.total_utilization:.5f}$"
        if condition1:
            st.success(message)
        else:
            st.error(message)

    condition2 = scheduler.condition2()
    icon_condition2 = "✅" if condition2 else "❌"
    with st.expander("Condition 2", expanded=False, icon=icon_condition2):
        st.write("The product of the utilizations must be less than or equal to 2.")
        st.write("- $\\prod_{i=1}^{n} (U_i + 1) \\leq 2$")
        st.write("- $U_i$ is the utilization of task $i$")

        product = scheduler.get_product()
        message = f"Product of utilizations is ${product:.3f}$"
        if condition2:
            st.success(message)
        else:
            st.error(message)

    response_time_analysis = ResponseTimeAnalysis.check_response_time(scheduler.tasks)
    icon_response_time_analysis = "✅" if response_time_analysis else "❌"
    with st.expander("Response Time Analysis", expanded=False, icon=icon_response_time_analysis):
        st.write("The response time analysis must be satisfied.")

        if response_time_analysis:
            st.success("Response time analysis is satisfied.")
        else:
            st.error("Response time analysis is not satisfied.")

        for task in scheduler.tasks:
            higher_priority_tasks = ResponseTimeAnalysis.get_higher_priority_tasks(task, scheduler.tasks)
            response_time = 0.0
            old_response_time = -1.0
            values: list[float] = []

            while response_time != old_response_time:
                old_response_time = response_time
                response_time = ResponseTimeAnalysis.iterate(response_time, task, higher_priority_tasks)
                values.append(response_time)

            iteration_data: list[str] = []

            if task.task_id == 1:
                values.pop()

            for i, val in enumerate(values):
                prefix = f"$W^{i}_{task.task_id}$"
                iteration_data.append(f"    - {prefix} = ${val:.5f}$\n")

            response_time = values[-1]
            icon_response_time = "✅" if response_time <= task.deadline else "❌"
            message = f"Response time is ${response_time:.3f}$"

            st.markdown(f"""
- Task ${task.task_id}$ (less than or equal to ${task.deadline}$):
{"\n".join(iteration_data)}
    - {icon_response_time} {message} (deadline is ${task.deadline}$)
                        """)

    with st.expander("Example Scheduling", expanded=False):
        st.write("#### Scheduling")

        scheduling = scheduler.get_scheduling()
        if scheduling.events is None:
            st.error("No scheduling found.")
            return

        for event in scheduling.events:
            st.write(f"""
- Task ${event.task.task_id}$:
    - Start Time: ${event.start_time}$
    - End Time: ${event.end_time}$
    - Compute Time: ${event.task.compute_time}$
                      """)

        additional_charts: list[alt.Chart] = []
        # Other charts

        create_chart(scheduling.events, additional_charts)
