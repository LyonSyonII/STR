import streamlit as st
from script.pages.utils.navigation import navigate_to

def run():
    st.title("Choose a Scheduler")

    st.button("Cyclic", on_click=navigate_to, args=("cyclic_scheduler",))
    st.button("Rate Monotonic", on_click=navigate_to, args=("rate_monotonic_scheduler",))
    st.button("Deadline Monotonic", on_click=navigate_to, args=("deadline_monotonic_scheduler",))
    st.button("Earliest Deadline First", on_click=navigate_to, args=("earliest_deadline_first_scheduler",))

    st.button("Back", on_click=navigate_to, args=("home",))
