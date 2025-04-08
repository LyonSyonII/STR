import streamlit as st
from script.pages.utils.navigation import navigate_to
from script.schedulers.cyclic import CyclicScheduler
from script.pages.utils.scheduler import show_basic_scheduler_info
from script.utils.task import SecondaryPeriod

def run():
    st.title("Cyclic Scheduler")

    st.button("Back", on_click=navigate_to, args=("home",))

    tasks = st.session_state.get("tasks", [])
    if not tasks:
        st.warning("Please add tasks first.")
        return

    scheduler = CyclicScheduler(tasks)
    show_basic_scheduler_info(scheduler)

    st.write("### Cyclic Scheduler Info")

    is_schedulable = scheduler.is_schedulable()
    if is_schedulable:
        st.success("The tasks are schedulable.")
    else:
        st.error("The tasks are not schedulable.")

    condition1 = scheduler.condition1()
    icon_condition1 = "✅" if condition1 else "❌"
    with st.expander("Condition 1", expanded=False, icon=icon_condition1):
        st.write("The total utilization must be less than or equal to 1.")

        condition1 = scheduler.condition1()
        message = f"Total utilization is ${scheduler.total_utilization:.5f}$"
        if condition1:
            st.success(message)
        else:
            st.error(message)

    condition2 = scheduler.condition2()
    icon_condition2 = "✅" if condition2 else "❌"
    with st.expander("Condition 2", expanded=False, icon=icon_condition2):
        if condition2:
            st.success("_Secondary Frame_ satisfies all conditions.")
        else:
            st.error("_Secondary Frame_ does not satisfy all conditions.")

        st.write(f"- Min deadline: ${scheduler.min_deadline}$")
        st.write(f"- Max compute time: ${scheduler.max_compute_time:.5f}$")

        ts = SecondaryPeriod.get_ts(
            scheduler.tasks,
            scheduler.min_deadline,
            scheduler.max_compute_time,
        )
        st.write(f"#### $T_s$ ({len(ts)} elements):")
        for _t in ts:
            st.write(f"- ${_t}$")

        st.write("#### Calculation of $T_s$")
        for _t in ts:
            st.markdown(f"##### $T_s$ = ${_t}$")

            for task in scheduler.tasks:
                condition_ts = SecondaryPeriod.compute_ts(_t, task.period)
                icon_condition_ts = "✅" if condition_ts <= task.deadline else "❌"
                k = scheduler.hyperperiod // _t

                st.markdown(f"""
    - Task {task.task_id}:
        - Compute Time: ${task.compute_time}$
        - **Deadline**: ${task.deadline}$
        - Period: ${task.period}$
        - {icon_condition_ts} $T_s$ **value** = ${condition_ts}$
        - $k$ = ${k}$
                            """)
