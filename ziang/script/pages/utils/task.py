import pandas as pd
import streamlit as st
from script.models.task import Task
from script.utils.task import hyperperiod, total_utilization

def show_tasks(tasks: list[Task]):
    """
    Displays the tasks in a table format.
    """
    if not tasks:
        st.warning("No tasks available.")
        return

    tasks_df = pd.DataFrame([task.__dict__ for task in tasks])
    st.dataframe(tasks_df, use_container_width=True)

def show_hyperperiod(tasks: list[Task]) -> None:
    """
    Display the hyperperiod of a set of tasks.

    :param tasks: The list of tasks.
    :type tasks: list[Task]
    """

    if not tasks:
        st.warning("No tasks available.")
        return

    hyperperiod_value = hyperperiod(tasks)
    with st.expander(f"Hyperperiod: ${hyperperiod_value}$"):
        for task in tasks:
            st.write(f"- Task {task.task_id}: Period = {task.period}")

        st.write(f"Hyperperiod Calculation: {hyperperiod_value}")

def show_total_utilization(tasks: list[Task]) -> None:
    """
    Display the total utilization of a set of tasks.

    :param tasks: The list of tasks.
    :type tasks: list[Task]
    """

    if not tasks:
        st.warning("No tasks available.")
        return

    utilization = total_utilization(tasks)
    with st.expander(f"Total Utilization: {utilization:.5%}"):
        for task in tasks:
            st.write(f"- Task {task.task_id}: Utilization = {task.compute_time / task.period:.5%}")

        st.write(f"Total Utilization Calculation: {utilization:.5%}")
