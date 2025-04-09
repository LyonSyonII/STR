import streamlit as st
from script.models.scheduler import Scheduler

def navigate_to(page):
    st.session_state["page"] = page

def navigate_to_scheduler(scheduler: type[Scheduler]):
    scheduler_name = scheduler.__name__
    st.session_state["page"] = scheduler_name
    navigate_to(scheduler_name)
