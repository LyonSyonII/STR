import streamlit as st

def navigate_to(page):
    st.session_state["page"] = page
