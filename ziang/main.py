import streamlit as st
from script.pages import home, editor, upload
from script.pages.schedulers import cyclic_scheduler, deadline_monotonic_scheduler, earliest_deadline_first_scheduler, rate_monotonic_scheduler

if "page" not in st.session_state:
    st.session_state["page"] = "home"

if "tasks" not in st.session_state:
    st.session_state["tasks"] = None

def main():
    match (st.session_state["page"]):
        case "home":
            home.run()

        case "editor":
            editor.run()

        case "upload":
            upload.run()

        case "cyclic_scheduler" | "CyclicScheduler":
            cyclic_scheduler.run()

        case "rate_monotonic_scheduler" | "RateMonotonicScheduler":
            rate_monotonic_scheduler.run()

        case "deadline_monotonic_scheduler" | "DeadlineMonotonicScheduler":
            deadline_monotonic_scheduler.run()

        case "earliest_deadline_first_scheduler" | "EarliestDeadlineFirstScheduler":
            earliest_deadline_first_scheduler.run()

        case _:
            st.error("Page not found")

if __name__ == "__main__":
    main()
