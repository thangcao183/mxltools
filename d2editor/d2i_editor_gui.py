#!/usr/bin/env python3
"""
D2I Item Property Editor - Complete GUI
========================================
Full-featured GUI for viewing and editing Diablo 2 item properties using the complete parser.

Features:
- Parse D2I files with full item structure support
- Display item information (type, quality, flags)
- View all properties with values and parameters
- Edit property values/parameters
- Add new properties
- Delete properties
- Save modified D2I files
"""

import tkinter as tk
from tkinter import ttk, filedialog, messagebox
import sqlite3
import os
from typing import Dict, List, Optional

from d2i_full_parser import D2ItemParser, ParsedItem, ItemProperty, ItemQuality
import property_inserter  # Import module mới


class D2IEditor:
    """Main GUI application for D2I property editing"""
    
    def __init__(self, root):
        self.root = root
        self.root.title("D2I Item Property Editor")
        self.root.geometry("1200x800")
        
        # Data
        self.property_db = {}
        self.skill_db = {}
        self.current_item: Optional[ParsedItem] = None
        self.current_file = ""
        self.parser: Optional[D2ItemParser] = None
        self.file_label = None  # Will be created in create_ui
        
        # Load databases
        self.load_databases()
        
        # Create parser
        self.parser = D2ItemParser(self.property_db)
        
        # Create UI
        self.create_menu()
        self.create_ui()
        
    def load_databases(self):
        """Load property and skill databases"""
        db_path = "data/props.db"
        if not os.path.exists(db_path):
            messagebox.showerror("Error", f"Database not found: {db_path}")
            return
        
        conn = sqlite3.connect(db_path)
        cursor = conn.cursor()
        
        # Load properties
        cursor.execute("SELECT code, name, h_descNegative, h_descStringAdd, addv, bits, paramBits, h_saveParamBits FROM props WHERE bits > 0")
        for row in cursor.fetchall():
            code, name, h_descNegative, h_descStringAdd,addv, bits, param_bits, h_save_param_bits = row
            
            # Clean up empty strings
            if param_bits == '':
                param_bits = None
            if h_save_param_bits == '':
                h_save_param_bits = None
            
            self.property_db[code] = {
                'name': name or f'prop_{code}',
                'desc': h_descNegative + " " + h_descStringAdd or '',
                'addv': addv if addv is not None else 0,
                'bits': bits,
                'paramBits': param_bits,
                'h_saveParamBits': h_save_param_bits
            }
        
        # Load skills (for param lookup)
        try:
            cursor.execute("SELECT code, name FROM skills")
            for row in cursor.fetchall():
                skill_id, skill_name = row
                self.skill_db[skill_id] = skill_name
        except Exception as e:
            print(f"Warning: Could not load skills table: {e}")
            pass  # Skills table may not exist
        
        conn.close()
        print(f"✅ Loaded {len(self.property_db)} properties, {len(self.skill_db)} skills")
    
    def create_menu(self):
        """Create menu bar"""
        menubar = tk.Menu(self.root)
        self.root.config(menu=menubar)
        
        # File menu
        file_menu = tk.Menu(menubar, tearoff=0)
        menubar.add_cascade(label="File", menu=file_menu)
        file_menu.add_command(label="Open D2I...", command=self.open_file, accelerator="Ctrl+O")
        file_menu.add_command(label="Save", command=self.save_file, accelerator="Ctrl+S")
        file_menu.add_command(label="Save As...", command=self.save_file_as, accelerator="Ctrl+Shift+S")
        file_menu.add_separator()
        file_menu.add_command(label="Exit", command=self.root.quit)
        
        # Bind keyboard shortcuts
        self.root.bind('<Control-o>', lambda e: self.open_file())
        self.root.bind('<Control-s>', lambda e: self.save_file())
        self.root.bind('<Control-Shift-S>', lambda e: self.save_file_as())
    
    def create_ui(self):
        """Create main UI"""
        # Create main container
        main_frame = ttk.Frame(self.root, padding=10)
        main_frame.pack(fill=tk.BOTH, expand=True)
        
        # Top: File selection panel
        file_frame = ttk.Frame(main_frame)
        file_frame.pack(fill=tk.X, pady=(0, 10))
        
        ttk.Button(file_frame, text="Load D2I File", command=self.open_file).pack(side=tk.LEFT, padx=5)
        self.file_label = ttk.Label(file_frame, text="No file loaded", foreground="gray")
        self.file_label.pack(side=tk.LEFT, padx=10)
        
        # Item info panel
        info_frame = ttk.LabelFrame(main_frame, text="Item Information", padding=10)
        info_frame.pack(fill=tk.X, pady=(0, 10))
        
        # Item info labels
        self.info_text = tk.Text(info_frame, height=6, wrap=tk.WORD, bg="#f0f0f0", fg="#000000")
        self.info_text.pack(fill=tk.BOTH)
        self.info_text.insert("1.0", "No file loaded. Use File > Open D2I to load an item.")
        self.info_text.config(state=tk.DISABLED)
        
        # Middle: Properties panel with buttons
        props_frame = ttk.LabelFrame(main_frame, text="Properties", padding=10)
        props_frame.pack(fill=tk.BOTH, expand=True)
        
        # Button panel
        button_frame = ttk.Frame(props_frame)
        button_frame.pack(fill=tk.X, pady=(0, 5))
        
        ttk.Button(button_frame, text="Add Property", command=self.add_property).pack(side=tk.LEFT, padx=2)
        ttk.Button(button_frame, text="Edit Selected", command=self.edit_property).pack(side=tk.LEFT, padx=2)
        ttk.Button(button_frame, text="Delete Selected", command=self.delete_property).pack(side=tk.LEFT, padx=2)
        ttk.Button(button_frame, text="Insert Property", command=self.insert_property).pack(side=tk.LEFT, padx=2)  # Nút mới

        # Properties tree
        tree_frame = ttk.Frame(props_frame)
        tree_frame.pack(fill=tk.BOTH, expand=True)
        
        # Scrollbars
        vsb = ttk.Scrollbar(tree_frame, orient="vertical")
        hsb = ttk.Scrollbar(tree_frame, orient="horizontal")
        
        # Tree
        self.tree = ttk.Treeview(tree_frame, 
                                 columns=("ID", "Name", "Desc","Value", "Param", "Skill"),
                                 yscrollcommand=vsb.set,
                                 xscrollcommand=hsb.set)
        
        vsb.config(command=self.tree.yview)
        hsb.config(command=self.tree.xview)
        
        # Grid layout
        self.tree.grid(row=0, column=0, sticky="nsew")
        vsb.grid(row=0, column=1, sticky="ns")
        hsb.grid(row=1, column=0, sticky="ew")
        
        tree_frame.grid_rowconfigure(0, weight=1)
        tree_frame.grid_columnconfigure(0, weight=1)
        
        # Configure columns
        self.tree.heading("#0", text="#")
        self.tree.heading("ID", text="Prop ID")
        self.tree.heading("Name", text="Property Name")
        self.tree.heading("Desc", text="Desc")
        self.tree.heading("Value", text="Value")
        self.tree.heading("Param", text="Parameter")
        self.tree.heading("Skill", text="Skill/Aura Name")
        
        self.tree.column("#0", width=50)
        self.tree.column("ID", width=70)
        self.tree.column("Name", width=300)
        self.tree.column("Desc", width=300)
        self.tree.column("Value", width=80)
        self.tree.column("Param", width=100)
        self.tree.column("Skill", width=250)
        
        # Double-click to edit
        self.tree.bind("<Double-Button-1>", lambda e: self.edit_property())
    
    def open_file(self):
        """Open and parse a D2I file"""
        filename = filedialog.askopenfilename(
            title="Select D2I file",
            filetypes=[("D2I files", "*.d2i"), ("All files", "*.*")]
        )
        
        if not filename:
            return
        
        try:
            # Parse file
            self.current_item = self.parser.parse_file(filename)
            self.current_file = filename
            
            # Update UI
            self.file_label.config(text=os.path.basename(filename), foreground="blue")
            self.update_item_info()
            self.update_properties_tree()
            
            self.root.title(f"D2I Item Property Editor - {os.path.basename(filename)}")
            
        except Exception as e:
            messagebox.showerror("Parse Error", f"Failed to parse file:\n{str(e)}")
            import traceback
            traceback.print_exc()
    
    def update_item_info(self):
        """Update item information display"""
        if not self.current_item:
            return
        
        item = self.current_item
        
        info_lines = [
            f"File: {os.path.basename(self.current_file)}",
            f"Item Type: {item.item_type}",
            f"Quality: {item.quality.name if item.is_extended else 'Normal (Simple Item)'}",
            f"Flags: {'Identified' if item.is_identified else 'Unidentified'}"
                   f"{', Ethereal' if item.is_ethereal else ''}"
                   f"{', Socketed (' + str(item.num_sockets) + ')' if item.is_socketed else ''}"
                   f"{', Runeword' if item.is_runeword else ''}"
                   f"{', Personalized' if item.is_personalized else ''}",
            f"Properties: {len(item.properties)} item"
                   f"{' + ' + str(len(item.runeword_properties)) + ' runeword' if item.runeword_properties else ''}"
        ]
        
        if item.is_extended:
            info_lines.append(f"Extended Data: ilvl={item.ilvl}, guid={item.guid}")
            if item.quality in [ItemQuality.Set, ItemQuality.Unique]:
                info_lines.append(f"Set/Unique ID: {item.set_or_unique_id}")
        
        self.info_text.config(state=tk.NORMAL)
        self.info_text.delete("1.0", tk.END)
        self.info_text.insert("1.0", "\n".join(info_lines))
        self.info_text.config(state=tk.DISABLED)
    
    def update_properties_tree(self):
        """Update properties tree view"""
        # Clear tree
        for item in self.tree.get_children():
            self.tree.delete(item)
        
        if not self.current_item:
            return
        
        # Add item properties
        for idx, prop in enumerate(self.current_item.properties):
            self.add_property_to_tree(idx + 1, prop, "item")
        
        # Add runeword properties
        if self.current_item.runeword_properties:
            for idx, prop in enumerate(self.current_item.runeword_properties):
                self.add_property_to_tree(
                    len(self.current_item.properties) + idx + 1, 
                    prop, 
                    "runeword"
                )
    
    def add_property_to_tree(self, index: int, prop: ItemProperty, prop_type: str):
        """Add a single property to the tree"""
        prop_info = self.property_db.get(prop.prop_id, {})
        prop_name = prop_info.get('name', f'Unknown_{prop.prop_id}')
        prop_desc = prop_info.get('desc', '')
        
        # Get skill name if param is set
        skill_name = ""
        if prop.param > 0 and prop.param in self.skill_db:
            skill_name = self.skill_db[prop.param]
        
        # Format parameter
        param_str = str(prop.param) if prop.param > 0 else ""
        
        # Add to tree
        tag = prop_type
        self.tree.insert("", tk.END, 
                        text=str(index),
                        values=(prop.prop_id, prop_name, prop_desc, prop.value, param_str, skill_name),
                        tags=(tag,))
        
        # Color runeword properties differently
        if prop_type == "runeword":
            self.tree.tag_configure("runeword", background="#ffe6cc")
    
    def add_property(self):
        """Add a new property"""
        if not self.current_item:
            messagebox.showwarning("No File", "Please open a D2I file first")
            return
        
        # Create dialog
        dialog = PropertyEditDialog(self.root, self.property_db, self.skill_db, title="Add Property")
        
        if dialog.result:
            prop_id, value, param = dialog.result
            
            try:
                # Add property using parser
                self.parser.add_property_to_item(self.current_item, prop_id, value, param)
                
                # Update tree
                self.update_properties_tree()
                self.update_item_info()
                
                messagebox.showinfo("Success", f"Property {prop_id} added successfully!")
                
            except Exception as e:
                messagebox.showerror("Error", f"Failed to add property:\n{str(e)}")
                import traceback
                traceback.print_exc()
    
    def edit_property(self):
        """Edit selected property"""
        selection = self.tree.selection()
        if not selection:
            messagebox.showwarning("No Selection", "Please select a property to edit")
            return

        item_id = selection[0]
        index = int(self.tree.item(item_id, "text")) - 1

        # Get property (check if runeword or item)
        if index < len(self.current_item.properties):
            prop = self.current_item.properties[index]
            is_runeword = False
        else:
            prop = self.current_item.runeword_properties[index - len(self.current_item.properties)]
            is_runeword = True

        # Create edit dialog
        dialog = PropertyEditDialog(
            self.root, 
            self.property_db, 
            self.skill_db,
            title="Edit Property",
            initial_prop_id=prop.prop_id,
            initial_value=prop.value,
            initial_param=prop.param
        )

        if dialog.result:
            prop_id, value, param = dialog.result

            try:
                # If the property ID didn't change, modify value/param in-place
                original_id = prop.prop_id
                if prop_id == original_id:
                    # Modify only value/param at the exact property index
                    self.parser.modify_property(self.current_item, index, new_value=value, new_param=param)
                else:
                    # ID changed: remove the old property and insert a new one
                    self.parser.delete_property_from_item(self.current_item, index)
                    self.parser.add_property_to_item(self.current_item, prop_id, value, param)

                # Update tree and info
                self.update_properties_tree()
                self.update_item_info()

                messagebox.showinfo("Success", f"Property {prop_id} updated successfully!")

            except Exception as e:
                messagebox.showerror("Error", f"Failed to update property:\n{str(e)}")
                import traceback
                traceback.print_exc()
    
    def delete_property(self):
        """Delete selected property"""
        selection = self.tree.selection()
        if not selection:
            messagebox.showwarning("No Selection", "Please select a property to delete")
            return
        
        if not messagebox.askyesno("Confirm Delete", "Are you sure you want to delete this property?"):
            return
        
        item_id = selection[0]
        index = int(self.tree.item(item_id, "text")) - 1
        
        try:
            # Delete property using parser
            if index < len(self.current_item.properties):
                self.parser.delete_property_from_item(self.current_item, index)
            else:
                # Runeword property
                rw_index = index - len(self.current_item.properties)
                # TODO: implement runeword property deletion
                messagebox.showwarning("Not Implemented", "Runeword property deletion not yet implemented")
                return
            
            # Update UI
            self.update_properties_tree()
            self.update_item_info()
            
            messagebox.showinfo("Success", "Property deleted successfully!")
            
        except Exception as e:
            messagebox.showerror("Error", f"Failed to delete property:\n{str(e)}")
            import traceback
            traceback.print_exc()
    
    def save_file(self):
        """Save current file"""
        if not self.current_file:
            self.save_file_as()
            return
        
        self.do_save(self.current_file)
    
    def save_file_as(self):
        """Save as new file"""
        filename = filedialog.asksaveasfilename(
            title="Save D2I file",
            defaultextension=".d2i",
            filetypes=[("D2I files", "*.d2i"), ("All files", "*.*")]
        )
        
        if filename:
            self.do_save(filename)
            self.current_file = filename
            self.root.title(f"D2I Item Property Editor - {os.path.basename(filename)}")
    
    def do_save(self, filename: str):
        """Actually save the file"""
        if not self.current_item:
            return

        try:
            # Save using parser
            self.parser.save_file(self.current_item, filename)
            messagebox.showinfo("Success", f"File saved successfully to:\n{filename}")

        except Exception as e:
            messagebox.showerror("Save Error", f"Failed to save file:\n{str(e)}")
            import traceback
            traceback.print_exc()
    
    def insert_property(self):
        """Insert a new property into the item bitstring"""
        if not self.current_item:
            messagebox.showwarning("No File", "Please open a D2I file first")
            return

        # Hiển thị hộp thoại để nhập thông tin property
        dialog = PropertyEditDialog(self.root, self.property_db, self.skill_db, title="Insert Property")

        if dialog.result:
            prop_id, value, param = dialog.result

            try:
                # Chèn property vào chuỗi bit
                updated_bitstring = property_inserter.insert_property(
                    self.current_item.bitstring, prop_id, value, self.property_db[prop_id]['bits']
                )

                # Cập nhật chuỗi bit của item
                self.current_item.bitstring = updated_bitstring

                # Cập nhật giao diện
                self.update_properties_tree()
                self.update_item_info()

                messagebox.showinfo("Success", f"Property {prop_id} inserted successfully!")

            except Exception as e:
                messagebox.showerror("Error", f"Failed to insert property:\n{str(e)}")
                import traceback
                traceback.print_exc()


class PropertyEditDialog:
    """Dialog for adding/editing a property"""
    
    def __init__(self, parent, property_db, skill_db, title="Edit Property",
                 initial_prop_id=None, initial_value=0, initial_param=0):
        self.result = None
        self.property_db = property_db
        self.skill_db = skill_db
        
        # Create dialog
        self.dialog = tk.Toplevel(parent)
        self.dialog.title(title)
        self.dialog.geometry("500x300")
        self.dialog.transient(parent)
        
        # Delay grab_set to avoid "window not viewable" error
        def set_grab():
            try:
                self.dialog.grab_set()
            except:
                pass
        self.dialog.after(100, set_grab)
        
        # Create UI
        frame = ttk.Frame(self.dialog, padding=20)
        frame.pack(fill=tk.BOTH, expand=True)
        
        # Property selection
        ttk.Label(frame, text="Property:").grid(row=0, column=0, sticky=tk.W, pady=5)
        
        self.prop_var = tk.StringVar()
        prop_combo = ttk.Combobox(frame, textvariable=self.prop_var, width=50)
        
        # Build property list
        prop_list = []
        for prop_id, info in sorted(self.property_db.items()):
            prop_list.append(f"{prop_id}: {info['name']}")
        prop_combo['values'] = prop_list
        
        if initial_prop_id is not None:
            prop_info = self.property_db.get(initial_prop_id, {})
            prop_combo.set(f"{initial_prop_id}: {prop_info.get('name', 'Unknown')}")
        
        prop_combo.grid(row=0, column=1, pady=5, sticky=tk.EW)
        
        # Value
        ttk.Label(frame, text="Value:").grid(row=1, column=0, sticky=tk.W, pady=5)
        self.value_var = tk.IntVar(value=initial_value)
        ttk.Entry(frame, textvariable=self.value_var).grid(row=1, column=1, pady=5, sticky=tk.EW)
        
        # Parameter
        ttk.Label(frame, text="Parameter:").grid(row=2, column=0, sticky=tk.W, pady=5)
        self.param_var = tk.IntVar(value=initial_param)
        ttk.Entry(frame, textvariable=self.param_var).grid(row=2, column=1, pady=5, sticky=tk.EW)
        
        # Skill lookup (if parameter is set)
        ttk.Label(frame, text="Skill Name:").grid(row=3, column=0, sticky=tk.W, pady=5)
        self.skill_label = ttk.Label(frame, text="", foreground="blue")
        self.skill_label.grid(row=3, column=1, pady=5, sticky=tk.W)
        
        # Update skill name when param changes
        self.param_var.trace_add("write", self.update_skill_name)
        self.update_skill_name()
        
        # Buttons
        button_frame = ttk.Frame(frame)
        button_frame.grid(row=4, column=0, columnspan=2, pady=20)
        
        ttk.Button(button_frame, text="OK", command=self.ok).pack(side=tk.LEFT, padx=5)
        ttk.Button(button_frame, text="Cancel", command=self.cancel).pack(side=tk.LEFT, padx=5)
        
        frame.columnconfigure(1, weight=1)
        
        # Wait for dialog
        self.dialog.wait_window()
    
    def update_skill_name(self, *args):
        """Update skill name label"""
        try:
            param = self.param_var.get()
            if param in self.skill_db:
                self.skill_label.config(text=self.skill_db[param])
            else:
                self.skill_label.config(text="")
        except:
            self.skill_label.config(text="")
    
    def ok(self):
        """OK button clicked"""
        try:
            # Parse property ID from combo
            prop_str = self.prop_var.get()
            if ':' in prop_str:
                prop_id = int(prop_str.split(':')[0])
            else:
                raise ValueError("Please select a property")
            
            value = self.value_var.get()
            param = self.param_var.get()
            
            self.result = (prop_id, value, param)
            self.dialog.destroy()
            
        except Exception as e:
            messagebox.showerror("Invalid Input", str(e), parent=self.dialog)
    
    def cancel(self):
        """Cancel button clicked"""
        self.dialog.destroy()


def main():
    """Main entry point"""
    root = tk.Tk()
    app = D2IEditor(root)
    root.mainloop()


if __name__ == "__main__":
    main()
