from typing import Optional
import streamlit as st

TOAST_TYPES = {
    "info": "info",
    "error": "error",
    "success": "success"
}

def show_toast(message: Optional[str] = None, toast_type: Optional[str] = None):
    """
    Show a toast message at the top of the page.
    """

    if message is None:
        message = st.session_state.get("toast_message", None)
    if toast_type is None:
        toast_type = st.session_state.get("toast_type", None)

    if toast_type == TOAST_TYPES["info"]:
        st.toast(message)
        st.info(message)
    elif toast_type == TOAST_TYPES["error"]:
        st.toast(message)
        st.error(message)
    elif toast_type == TOAST_TYPES["success"]:
        st.toast(message)
        st.success(message)

    # Reset the toast type after displaying it
    st.session_state["toast_type"] = None
    st.session_state["toast_message"] = None
    st.session_state["show_info_toast"] = False
