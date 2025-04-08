from rich.console import Console
from rich.table import Table

from script.models.task import Task

def print_table(data: list[Task], title: str = "Task Table") -> None:
    """
    Print a table of tasks using Rich.

    :param data: The list of tasks to print.
    :type data: list[Task]
    :param title: The title of the table, defaults to "Task Table"
    :type title: str, optional
    """
    console = Console()
    table = Table(title=title)

    # Add columns to the table
    table.add_column("ID", justify="center", style="cyan", no_wrap=True)
    table.add_column("Period", justify="right", style="magenta")
    table.add_column("Compute Time", justify="right", style="green")
    table.add_column("Deadline", justify="right", style="blue")
    table.add_column("Priority", justify="right", style="yellow")

    # Add rows to the table
    for task in data:
        table.add_row(
            str(task.task_id),
            str(task.period),
            str(task.compute_time),
            str(task.deadline),
            str(task.priority),
        )

    console.print(table)
