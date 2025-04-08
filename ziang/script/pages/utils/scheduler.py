import streamlit as st
from script.models.scheduler import Scheduler
from script.pages.utils.task import show_tasks, show_hyperperiod, show_total_utilization

def show_basic_scheduler_info(scheduler: Scheduler):
    """
    Displays basic information about the scheduler.
    """
    st.write("### Scheduler Type")
    st.write(type(scheduler).__name__)

    st.write("### Tasks")
    st.write(f"With a total of {len(scheduler.tasks)} tasks.")
    show_tasks(scheduler.tasks)

    show_hyperperiod(scheduler.tasks)
    show_total_utilization(scheduler.tasks)
