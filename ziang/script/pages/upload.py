import streamlit as st
import pandas as pd
from script.pages.utils.file import read_tasks, DELIMITERS
from script.pages.utils.navigation import navigate_to, navigate_to_scheduler
from script.pages.utils.task import show_tasks
from script.pages.utils.toast import show_toast
from script.schedulers import SCHEDULERS

# compute_time, deadline = period
EXAMPLE_2_COLUMNS: list[list[float | int]] = [
    [1.0, 10],
    [2.0, 20],
    [3.0, 30],
]

# compute_time, deadline, period
EXAMPLE_3_COLUMNS: list[list[float | int]] = [
    [1.0, 10, 10],
    [2.0, 20, 20],
    [3.0, 30, 30],
]

# compute_time, deadline, period, priority
EXAMPLE_4_COLUMNS: list[list[float | int]] = [
    [1.0, 10, 10, 1],
    [2.0, 20, 20, 2],
    [3.0, 30, 30, 3],
]

# compute_time, deadline, period, priority, task_id
EXAMPLE_5_COLUMNS: list[list[float | int]] = [
    [1.0, 10, 10, 1, 1],
    [2.0, 20, 20, 2, 2],
    [3.0, 30, 30, 3, 3],
]

def run():
    st.title("Upload CSV file")

    st.button("Back", on_click=navigate_to, args=("home",))

    uploaded_file = st.file_uploader("Choose a CSV file", type="csv")

    if uploaded_file is None:
        return

    chosen_delimiter = st.selectbox("Select delimiter", DELIMITERS.keys(), index=0)
    delimiter = DELIMITERS[chosen_delimiter]

    st.divider()

    has_header = st.checkbox("Does the file have a header?", value=True)
    if has_header:
        st.info("The first row will be used as the header.")
    else:
        st.warning("The first row will **NOT** be used as the header.")

    st.divider()

    # A series of radio buttons to select the type of scheduler
    scheduler_type = st.radio(
        "Select the type of scheduler",
        options=list(SCHEDULERS.keys()),
        index=None,
    )

    if scheduler_type is not None:
        selected_scheduler = SCHEDULERS[scheduler_type]
        st.session_state["scheduler"] = selected_scheduler
        st.info(f"Scheduler selected: {scheduler_type}")

        st.divider()

        if st.button("Import"):
            try:
                tasks = read_tasks(uploaded_file, selected_scheduler, delimiter, has_header)

                show_toast("File imported successfully", "success")

                st.session_state["tasks"] = tasks

                st.subheader("Imported Tasks")
                st.write(f"Number of tasks: {len(tasks)}")

                st.write("Tasks:")
                show_tasks(tasks)

                st.button("Edit Data", on_click=navigate_to, args=("editor",))
                st.button("Continue", on_click=navigate_to_scheduler, args=(selected_scheduler,))

            except Exception as e:
                show_toast("Error importing file", "error")
                st.error(f"Error importing file: {e}")

                st.divider()

                st.subheader("Example CSV files")
                st.write("You can use the following examples as a reference:")

                st.write("1. 2 columns (compute_time, deadline)")
                st.write(pd.DataFrame(EXAMPLE_2_COLUMNS, columns=["compute_time", "deadline is the same as period"]))

                st.divider()

                st.write("2. 3 columns (compute_time, deadline, period)")
                st.write(pd.DataFrame(EXAMPLE_3_COLUMNS, columns=["compute_time", "deadline", "period"]))

                st.divider()

                st.write("3. 4 columns (compute_time, deadline, period, priority)")
                st.write(pd.DataFrame(EXAMPLE_4_COLUMNS, columns=["compute_time", "deadline", "period", "priority"]))

                st.divider()

                st.write("4. 5 columns (compute_time, deadline, period, priority, task_id)")
                st.write(pd.DataFrame(EXAMPLE_5_COLUMNS, columns=["compute_time", "deadline", "period", "priority", "task_id"]))
