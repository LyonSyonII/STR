from typing import Optional
import pandas as pd
import streamlit as st
from script.models.task import Task
from script.models.scheduler import Scheduler
from script.pages.utils.navigation import navigate_to, navigate_to_scheduler
from script.pages.utils.file import read_tasks_from_df

CONFIG_COLUMNS = {
    "compute_time": st.column_config.NumberColumn(label="Compute Time", min_value=0, required=True),
    "deadline": st.column_config.NumberColumn(label="Deadline", min_value=0, required=True),
    "period": st.column_config.NumberColumn(label="Period", min_value=0, required=True),
    "priority": st.column_config.NumberColumn(label="Priority", min_value=0, required=True),
    "task_id": st.column_config.NumberColumn(label="Task ID", min_value=0, required=False),
}

def _continue(scheduler: type[Scheduler]):
    """
    Continue to the next page without saving changes.
    """
    st.info("No changes saved")
    navigate_to_scheduler(scheduler)

def _save_changes(scheduler: type[Scheduler], new_tasks: Optional[list[Task]] = None):
    """
    Save changes to the session state and navigate to a different page.
    """
    if new_tasks is None:
        st.warning("No changes to save.")
    else:
        st.session_state["tasks"] = new_tasks

    navigate_to_scheduler(scheduler)

def _back_to_upload(navigate: str):
    """
    Navigate back to the upload page without saving changes.
    """
    st.warning("Uploading a new file")
    st.session_state["tasks"] = None
    navigate_to(navigate)

def _back(navigate: str):
    """
    Navigate back to the previous page without saving changes.
    """
    st.warning("No changes saved")
    navigate_to(navigate)

def run():
    st.title("Data Editor")

    st.button("Back", on_click=_back, args=("home",))
    st.button("Back to Upload", on_click=_back_to_upload, args=("upload",))

    tasks = st.session_state.get("tasks", None)
    if tasks is None:
        st.error("No data available to edit.")
        return

    tasks_df = pd.DataFrame(tasks)

    edited_df = st.data_editor(
        tasks_df,
        column_config=CONFIG_COLUMNS,
        hide_index=True,
        num_rows="dynamic",
    )

    scheduler = st.session_state.get("scheduler", None)
    if scheduler is None:
        st.error("No scheduler selected.")
        return

    edited_tasks = read_tasks_from_df(edited_df, scheduler)

    st.button("Continue", on_click=_continue, args=(scheduler,))
    st.button("Save Changes", on_click=_save_changes, args=(scheduler, edited_tasks, ))
