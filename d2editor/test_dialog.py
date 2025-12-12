
import tkinter as tk
from d2i_editor_gui import PropertyEditDialog

def test_dialog():
    root = tk.Tk()
    # Mock DBs
    prop_db = {0: {'name': 'Str', 'desc': '', 'bits': 9, 'addv': 0}, 1: {'name': 'Dex', 'desc': '', 'bits': 9, 'addv': 0}}
    skill_db = {}
    
    def on_click():
        dialog = PropertyEditDialog(root, prop_db, skill_db, allow_multiple=True)
        print(f"Result: {dialog.result}")
        
    tk.Button(root, text="Open Dialog", command=on_click).pack()
    
    # Auto-drive the dialog? Hard with Tkinter mainloop.
    # We can just verify the logic by inspecting the code or manual test.
    # But since I am an AI, I can't click.
    
    root.mainloop()

if __name__ == "__main__":
    test_dialog()
