from typing import Optional
import streamlit as st
import pandas as pd
import altair as alt
from altair import Undefined
from script.models.event import Event

def create_chart(events: list[Event], charts: Optional[list[alt.Chart]] = None):
    """
    Create a Gantt chart using Altair.
    """
    df = pd.DataFrame(
        [(event.task.task_id, event.start_time, event.end_time) for event in events],
        columns=["Task ID", "Start Time", "End Time"],
    )

    there_are_preemptions = any(len(event.interruptions) > 0 for event in events)
    expanded = st.toggle("Expand Chart", value=True) or there_are_preemptions

    chart = alt.Chart(df).mark_bar().encode(
        x=alt.X("Start Time:Q", title="Start Time"),
        x2=alt.X2("End Time:Q", title="End Time"),
        y=alt.Y("Task ID:N", title="Task ID") if expanded else Undefined,
        color=alt.Color("Task ID:N", legend=None),
    ).properties(
        width=800,
        height=400,
    ).interactive()

    if charts is not None:
        for c in charts:
            chart += c

    st.altair_chart(chart, use_container_width=True)
