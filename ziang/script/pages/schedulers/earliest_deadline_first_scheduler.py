from dataclasses import dataclass
import streamlit as st
import pandas as pd
import altair as alt
from script.pages.utils.navigation import navigate_to
from script.schedulers.earliest_deadline_first import EarliestDeadlineFirstScheduler
from script.pages.utils.scheduler import show_basic_scheduler_info
from script.pages.utils.chart import create_chart
from script.utils.task import ProcessorDemandCriterion

@dataclass
class TimeSlotData:
    time_slot: float
    g_value: float
    g_condition: bool

def run():
    st.title("Earliest Deadline First Scheduler")

    st.button("Back", on_click=navigate_to, args=("home",))

    tasks = st.session_state.get("tasks", [])
    if not tasks:
        st.warning("Please add tasks first.")
        return

    scheduler = EarliestDeadlineFirstScheduler(tasks)
    show_basic_scheduler_info(scheduler)

    st.write("### Earliest Deadline First Scheduler Info")

    is_schedulable = scheduler.is_schedulable()
    if is_schedulable:
        st.success("The tasks are schedulable.")
    else:
        st.error("The tasks are not schedulable.")

    condition1 = scheduler.condition1()
    icon_condition1 = "✅" if condition1 else "❌"
    with st.expander("Condition 1", expanded=False, icon=icon_condition1):
        st.write("Applies if all tasks have the same period and deadline.")
        st.write("And the total utilization is less than or equal to 1.")

        if condition1:
            st.success("Condition 1 is satisfied.")
        else:
            st.error("Condition 1 is not satisfied.")

        for task in scheduler.tasks:
            task_validity = task.period == task.deadline
            task_status = "✅ Valid Task" if task_validity else "❌ Not Valid Task"

            st.write(f"""
- Task ${task.task_id}$:
    - Period: ${task.period}$
    - Deadline: ${task.deadline}$
    - {task_status}
                      """)

        st.write(f"Total Utilization: ${scheduler.total_utilization:.2f}$")

    condition2 = scheduler.condition2()
    icon_condition2 = "✅" if condition2 else "❌"
    with st.expander("Condition 2", expanded=False, icon=icon_condition2):
        st.write("Applies if the tasks have different periods and deadlines.")
        st.write("It depends on the processor demand criterion.")

        if condition2:
            st.success("Condition 2 is satisfied.")
        else:
            st.error("Condition 2 is not satisfied.")

        st.write(f"Hyperperiod $H$: ${scheduler.hyperperiod:.2f}$")

        l_star = ProcessorDemandCriterion.get_l_star(scheduler.tasks)
        st.write(f"$L^*$: ${l_star:.2f}$")

        max_time = ProcessorDemandCriterion.get_max_time_slot(scheduler.tasks)
        st.write(f"Max Time Slot: ${max_time:.2f}$")

        time_slots = ProcessorDemandCriterion.get_time_slots(scheduler.tasks)
        st.write("Time Slots:")
        for time_slot in time_slots:
            st.write(f"- ${time_slot:.2f}$")

        time_slot_data: list[TimeSlotData] = []

        for time_slot in time_slots:
            g_value = ProcessorDemandCriterion.get_g(scheduler.tasks, 0, time_slot)
            g_condition = ProcessorDemandCriterion.check_g(scheduler.tasks, 0, time_slot)
            time_slot_data.append(TimeSlotData(time_slot, g_value, g_condition))

        # Display the data in a table
        df = pd.DataFrame(time_slot_data)
        df["g_condition"] = df["g_condition"].apply(lambda x: "✅" if x else "❌")
        df.columns = ["Time Slot", "g Value", "g Condition"]
        st.dataframe(df, use_container_width=True)

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
