import streamlit as st
import pandas as pd
from script.pages.utils.file import read_tasks, DELIMITERS
from script.pages.utils.navigation import navigate_to
from script.pages.utils.task import show_tasks
from script.pages.utils.toast import show_toast
from script.models.task import Task, AperiodicTask

EXAMPLE_TASKS: list[Task] = [
    Task(compute_time=1, deadline=5, period=10, task_id=1, priority=1),
    Task(compute_time=2, deadline=6, period=10, task_id=2, priority=2),
    Task(compute_time=3, deadline=7, period=10, task_id=3, priority=3),
]

EXAMPLE_APERIODIC_TASKS: list[AperiodicTask] = [
    AperiodicTask(compute_time=1, deadline=5, period=10, task_id=1, arrival_time=0, priority=1),
    AperiodicTask(compute_time=2, deadline=6, period=10, task_id=2, arrival_time=1, priority=2),
    AperiodicTask(compute_time=3, deadline=7, period=10, task_id=3, arrival_time=2, priority=3),
]

def run():
    st.title("Upload CSV file")

    uploaded_file = st.file_uploader("Choose a CSV file", type="csv")

    if uploaded_file is None:
        return

    chosen_delimiter = st.selectbox("Select delimiter", DELIMITERS.keys(), index=0)
    delimiter = DELIMITERS[chosen_delimiter]

    has_header = st.checkbox("Does the file have a header?", value=True)

    if st.button("Import"):
        try:
            tasks = read_tasks(uploaded_file, delimiter, has_header)

            show_toast("File imported successfully", "success")

            st.session_state["tasks"] = tasks

            st.subheader("Imported Tasks")
            st.write(f"Number of tasks: {len(tasks)}")

            st.write("Tasks:")
            show_tasks(tasks)

            st.button("Edit Data", on_click=navigate_to, args=("editor",))
            st.button("Continue", on_click=navigate_to, args=("schedulers_selector",))

        except Exception as e:
            show_toast("Error importing file", "error")
            st.error(f"Error importing file: {e}")

            st.subheader("Example CSV file format for tasks")
            example_df = pd.DataFrame([task.__dict__ for task in EXAMPLE_TASKS])
            st.table(example_df)

            st.subheader("Example CSV file format for aperiodic tasks")
            example_aperiodic_df = pd.DataFrame([task.__dict__ for task in EXAMPLE_APERIODIC_TASKS])
            st.table(example_aperiodic_df)

    st.button("Back", on_click=navigate_to, args=("home",))
