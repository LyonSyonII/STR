import streamlit as st
import altair as alt
from script.pages.utils.navigation import navigate_to
from script.schedulers.deadline_monotonic import DeadlineMonotonicScheduler
from script.pages.utils.scheduler import show_basic_scheduler_info
from script.pages.utils.chart import create_chart
from script.utils.task import ResponseTimeAnalysis


def run():
    st.title("Deadline Monotonic Scheduler")

    st.button("Back", on_click=navigate_to, args=("home",))

    tasks = st.session_state.get("tasks", [])
    if not tasks:
        st.warning("Please add tasks first.")
        return

    scheduler = DeadlineMonotonicScheduler(tasks)
    show_basic_scheduler_info(scheduler)

    st.write("### Deadline Monotonic Scheduler Info")

    is_schedulable = scheduler.is_schedulable()
    if is_schedulable:
        st.success("The tasks are schedulable.")
    else:
        st.error("The tasks are not schedulable.")

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
                iteration_data.append(f"    - {prefix} = ${val:.3f}$\n")

            response_time = values[-1]
            icon_response_time = "✅" if response_time <= task.deadline else "❌"
            message = f"Response time is ${response_time:.3f}$"

            st.markdown(f"""
- Task ${task.task_id}$ (less than or equal to ${task.deadline}$):
{"\n".join(iteration_data)}
    - {icon_response_time} {message} (deadline is ${task.deadline:.3f}$)
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
