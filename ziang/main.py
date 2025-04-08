import streamlit as st
from script.pages import home, editor, upload
from script.pages.schedulers import selector, cyclic_scheduler, deadline_monotonic_scheduler, earliest_deadline_first_scheduler, rate_monotonic_scheduler

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

        case "schedulers_selector":
            selector.run()

        case "cyclic_scheduler":
            cyclic_scheduler.run()

        case "rate_monotonic_scheduler":
            rate_monotonic_scheduler.run()

        case "deadline_monotonic_scheduler":
            deadline_monotonic_scheduler.run()

        case "earliest_deadline_first_scheduler":
            earliest_deadline_first_scheduler.run()

        case _:
            st.error("Page not found")

if __name__ == "__main__":
    main()
