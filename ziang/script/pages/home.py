import streamlit as st
from script.pages.utils.navigation import navigate_to

def run():
    st.session_state.pop("tasks", None)
    st.session_state.pop("page", None)

    st.title("STR: Schedulers")
    st.button("Upload Data", on_click=navigate_to, args=("upload",))
