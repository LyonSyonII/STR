import streamlit as st
from script.pages.utils.navigation import navigate_to
from script.schedulers.earliest_deadline_first import EarliestDeadlineFirstScheduler
from script.pages.utils.scheduler import show_basic_scheduler_info
from script.utils.task import ProcessorDemandCriterion

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
            st.write(f"- Task ${task.task_id}$:")
            st.write(f"    - Period: ${task.period}$")
            st.write(f"    - Deadline: ${task.deadline}$")

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

        max_time = ProcessorDemandCriterion.get_max_time_slot(scheduler.tasks)
        st.write(f"Max Time Slot: ${max_time:.2f}$")
