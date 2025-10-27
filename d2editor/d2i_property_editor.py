#!/usr/bin/env python3
"""
D2I Property Editor GUI
========================
A comprehensive GUI application to view, edit, add, and delete properties in D2I item files.
Supports both standard properties and parameterized properties (with skill/aura IDs).

Features:
- Load and parse D2I files
- Display all existing properties with their values and parameters
- Edit property values and parameters
- Add new properties (standard and parameterized)
- Delete existing properties
- Save modified D2I files
- Skill/Aura name lookup from database
"""

import tkinter as tk
from tkinter import ttk, filedialog, messagebox, scrolledtext
import sqlite3
import os
from typing import Dict, List, Tuple, Optional
from property_adder import (
    PropertyAdder,
    PROPERTIES,
    create_bitstring,
    find_all_end_markers
)
from bitutils import number_to_binary_lsb, bitstring_to_bytes


class PropertyInfo:
    """Information about a property extracted from D2I file"""
    def __init__(self, prop_id: int, value: int, param: Optional[int] = None, 
                 bit_position: int = 0):
        self.prop_id = prop_id
        self.value = value
        self.param = param
        self.bit_position = bit_position
        self.name = ""
        self.value_bits = 0
        self.param_bits = 0
        self.h_save_param_bits = 0
        self.addv = 0
        
    def __repr__(self):
        if self.param is not None:
            return f"Property({self.prop_id}, value={self.value}, param={self.param})"
        return f"Property({self.prop_id}, value={self.value})"


class D2IPropertyEditor:
    """Main GUI application for editing D2I properties"""
    
    def __init__(self, root):
        self.root = root
        self.root.title("D2I Property Editor - MedianXL Offline Tools")
        self.root.geometry("1200x800")
        
        # Data
        self.current_file = None
        self.properties_list: List[PropertyInfo] = []
        self.property_db: Dict = {}
        self.skills_db: Dict = {}
        self.original_bitstring = ""
        self.end_marker_pos = 0
        
        # Load databases
        self.load_databases()
        
        # Create UI
        self.create_ui()
        
    def load_databases(self):
        """Load property and skill databases"""
        db_path = "data/props.db"
        if not os.path.exists(db_path):
            messagebox.showerror("Error", f"Database not found: {db_path}")
            return
            
        try:
            conn = sqlite3.connect(db_path)
            cursor = conn.cursor()
            
            # Load properties
            cursor.execute("""
                SELECT code, name, addv, bits, paramBits, h_saveParamBits 
                FROM props 
                WHERE bits > 0
            """)
            
            for row in cursor.fetchall():
                code, name, addv, bits, param_bits, h_save_param_bits = row
                
                # Clean up empty strings and None values
                if param_bits == '':
                    param_bits = None
                if h_save_param_bits == '':
                    h_save_param_bits = None
                
                self.property_db[code] = {
                    'name': name or f'prop_{code}',
                    'addv': addv if addv is not None else 0,
                    'bits': bits,
                    'paramBits': param_bits,
                    'h_saveParamBits': h_save_param_bits
                }
            
            # Load skills
            cursor.execute("SELECT code, name, class FROM skills")
            for row in cursor.fetchall():
                code, name, skill_class = row
                self.skills_db[code] = {
                    'name': name or f'Skill {code}',
                    'class': skill_class if skill_class is not None else -1
                }
            
            conn.close()
            print(f"✅ Loaded {len(self.property_db)} properties and {len(self.skills_db)} skills")
            
        except Exception as e:
            messagebox.showerror("Database Error", f"Failed to load database: {e}")
    
    def create_ui(self):
        """Create the user interface"""
        
        # Menu bar
        menubar = tk.Menu(self.root)
        self.root.config(menu=menubar)
        
        file_menu = tk.Menu(menubar, tearoff=0)
        menubar.add_cascade(label="File", menu=file_menu)
        file_menu.add_command(label="Open D2I File...", command=self.open_file)
        file_menu.add_command(label="Save As...", command=self.save_file)
        file_menu.add_separator()
        file_menu.add_command(label="Exit", command=self.root.quit)
        
        help_menu = tk.Menu(menubar, tearoff=0)
        menubar.add_cascade(label="Help", menu=help_menu)
        help_menu.add_command(label="About", command=self.show_about)
        
        # Top frame - File info
        top_frame = ttk.Frame(self.root, padding="10")
        top_frame.pack(fill=tk.X)
        
        ttk.Label(top_frame, text="File:", font=('Arial', 10, 'bold')).pack(side=tk.LEFT)
        self.file_label = ttk.Label(top_frame, text="No file loaded", 
                                     foreground="gray")
        self.file_label.pack(side=tk.LEFT, padx=10)
        
        ttk.Button(top_frame, text="Open File", 
                   command=self.open_file).pack(side=tk.RIGHT, padx=5)
        
        # Main container with two panes
        paned = ttk.PanedWindow(self.root, orient=tk.HORIZONTAL)
        paned.pack(fill=tk.BOTH, expand=True, padx=10, pady=5)
        
        # Left pane - Properties list
        left_frame = ttk.Frame(paned)
        paned.add(left_frame, weight=1)
        
        ttk.Label(left_frame, text="Properties in Item:", 
                  font=('Arial', 11, 'bold')).pack(anchor=tk.W, pady=5)
        
        # Properties treeview
        tree_frame = ttk.Frame(left_frame)
        tree_frame.pack(fill=tk.BOTH, expand=True)
        
        scrollbar = ttk.Scrollbar(tree_frame)
        scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
        
        self.tree = ttk.Treeview(tree_frame, columns=('ID', 'Name', 'Value', 'Param', 'Display'),
                                 show='headings', yscrollcommand=scrollbar.set)
        scrollbar.config(command=self.tree.yview)
        
        self.tree.heading('ID', text='ID')
        self.tree.heading('Name', text='Property Name')
        self.tree.heading('Value', text='Raw Value')
        self.tree.heading('Param', text='Parameter')
        self.tree.heading('Display', text='Display Info')
        
        self.tree.column('ID', width=50, anchor=tk.CENTER)
        self.tree.column('Name', width=180)
        self.tree.column('Value', width=80, anchor=tk.CENTER)
        self.tree.column('Param', width=80, anchor=tk.CENTER)
        self.tree.column('Display', width=200)
        
        self.tree.pack(fill=tk.BOTH, expand=True)
        self.tree.bind('<<TreeviewSelect>>', self.on_property_select)
        
        # Buttons below tree
        btn_frame = ttk.Frame(left_frame)
        btn_frame.pack(fill=tk.X, pady=10)
        
        ttk.Button(btn_frame, text="Add Property", 
                   command=self.add_property).pack(side=tk.LEFT, padx=5)
        ttk.Button(btn_frame, text="Delete Selected", 
                   command=self.delete_property).pack(side=tk.LEFT, padx=5)
        ttk.Button(btn_frame, text="Refresh", 
                   command=self.refresh_properties).pack(side=tk.LEFT, padx=5)
        
        # Right pane - Property editor
        right_frame = ttk.Frame(paned)
        paned.add(right_frame, weight=1)
        
        ttk.Label(right_frame, text="Edit Property:", 
                  font=('Arial', 11, 'bold')).pack(anchor=tk.W, pady=5)
        
        # Editor form
        form_frame = ttk.LabelFrame(right_frame, text="Property Details", padding="10")
        form_frame.pack(fill=tk.BOTH, expand=True, pady=5)
        
        # Property ID
        row = 0
        ttk.Label(form_frame, text="Property ID:").grid(row=row, column=0, sticky=tk.W, pady=5)
        self.prop_id_var = tk.StringVar()
        self.prop_id_entry = ttk.Entry(form_frame, textvariable=self.prop_id_var, 
                                        state='readonly', width=15)
        self.prop_id_entry.grid(row=row, column=1, sticky=tk.W, padx=5, pady=5)
        
        # Property Name
        row += 1
        ttk.Label(form_frame, text="Name:").grid(row=row, column=0, sticky=tk.W, pady=5)
        self.prop_name_var = tk.StringVar()
        ttk.Entry(form_frame, textvariable=self.prop_name_var, 
                  state='readonly', width=30).grid(row=row, column=1, sticky=tk.W, padx=5, pady=5)
        
        # Value
        row += 1
        ttk.Label(form_frame, text="Value (raw):").grid(row=row, column=0, sticky=tk.W, pady=5)
        self.value_var = tk.StringVar()
        self.value_entry = ttk.Entry(form_frame, textvariable=self.value_var, width=15)
        self.value_entry.grid(row=row, column=1, sticky=tk.W, padx=5, pady=5)
        
        row += 1
        ttk.Label(form_frame, text="Display value:").grid(row=row, column=0, sticky=tk.W, pady=5)
        self.display_value_var = tk.StringVar()
        ttk.Entry(form_frame, textvariable=self.display_value_var, 
                  state='readonly', width=15).grid(row=row, column=1, sticky=tk.W, padx=5, pady=5)
        
        # Parameter
        row += 1
        ttk.Label(form_frame, text="Parameter ID:").grid(row=row, column=0, sticky=tk.W, pady=5)
        self.param_var = tk.StringVar()
        self.param_entry = ttk.Entry(form_frame, textvariable=self.param_var, width=15)
        self.param_entry.grid(row=row, column=1, sticky=tk.W, padx=5, pady=5)
        
        row += 1
        ttk.Label(form_frame, text="Parameter name:").grid(row=row, column=0, sticky=tk.W, pady=5)
        self.param_name_var = tk.StringVar()
        ttk.Entry(form_frame, textvariable=self.param_name_var, 
                  state='readonly', width=30).grid(row=row, column=1, sticky=tk.W, padx=5, pady=5)
        
        # Bit info
        row += 1
        ttk.Separator(form_frame, orient=tk.HORIZONTAL).grid(row=row, column=0, 
                                                              columnspan=2, sticky=tk.EW, pady=10)
        
        row += 1
        ttk.Label(form_frame, text="Bit Structure:", 
                  font=('Arial', 9, 'bold')).grid(row=row, column=0, columnspan=2, sticky=tk.W, pady=5)
        
        row += 1
        self.bit_info_text = scrolledtext.ScrolledText(form_frame, height=6, width=50, 
                                                        font=('Courier', 9), state='disabled')
        self.bit_info_text.grid(row=row, column=0, columnspan=2, sticky=tk.EW, pady=5)

        # Apply button
        row += 1
        ttk.Button(form_frame, text="Apply Changes",
                   command=self.apply_changes).grid(row=row, column=0, columnspan=1, pady=15)
        # Max button: set the value entry to the maximum possible storage value (displayed value = storage - addv)
        ttk.Button(form_frame, text="Max", command=self.set_selected_prop_max).grid(row=row, column=1, pady=15, sticky=tk.W)

        # Status bar
        status_frame = ttk.Frame(self.root)
        status_frame.pack(fill=tk.X, side=tk.BOTTOM)
        
        self.status_var = tk.StringVar(value="Ready")
        ttk.Label(status_frame, textvariable=self.status_var, 
                  relief=tk.SUNKEN, anchor=tk.W).pack(fill=tk.X, padx=5, pady=2)
        
    def open_file(self):
        """Open and parse a D2I file"""
        filename = filedialog.askopenfilename(
            title="Open D2I Item File",
            filetypes=[("D2I Files", "*.d2i"), ("All Files", "*.*")],
            initialdir="d2i"
        )
        
        if not filename:
            return
            
        try:
            self.current_file = filename
            self.file_label.config(text=os.path.basename(filename), foreground="black")
            
            # Parse the file
            self.parse_d2i_file(filename)
            
            # Update UI
            self.refresh_properties()
            self.status_var.set(f"Loaded: {filename} - {len(self.properties_list)} properties found")
            
        except Exception as e:
            messagebox.showerror("Error", f"Failed to load file: {e}")
            import traceback
            traceback.print_exc()
    
    def parse_d2i_file(self, filename: str):
        """Parse D2I file and extract all properties"""
        self.properties_list.clear()
        
        # Load file using PropertyAdder
        adder = PropertyAdder(filename)
        if not adder.load_file():
            raise ValueError("Failed to load D2I file")
        
        self.original_bitstring = adder.original_bitstring
        self.end_marker_pos = adder.end_marker_pos
        
        # Extract properties from bitstring
        bitstring = self.original_bitstring[:self.end_marker_pos]
        
        pos = 0
        while pos < len(bitstring) - 9:  # Need at least 9 bits for property ID
            # Read property ID (9 bits, LSB-first)
            if pos + 9 > len(bitstring):
                break
                
            prop_id_bits = bitstring[pos:pos+9]
            prop_id = self.bits_to_number_lsb(prop_id_bits)
            pos += 9
            
            # Check if end marker
            if prop_id == 0x1FF:  # End marker
                break
            
            # Get property info from database
            if prop_id not in self.property_db:
                print(f"⚠️  Unknown property ID {prop_id} at position {pos-9}, skipping...")
                break
            
            prop_info = self.property_db[prop_id]
            
            # Check if has parameter - MATCH C++ LOGIC: txtProperty->paramBits ? ... : 0
            # Database returns TEXT type for paramBits/h_saveParamBits, need to parse carefully
            param_bits = 0
            
            # Try h_saveParamBits first (for runeword properties like prop 97)
            h_save = prop_info['h_saveParamBits']
            if h_save is not None:
                h_save_str = str(h_save).strip()
                if h_save_str and h_save_str != '' and h_save_str.isdigit():
                    param_bits = int(h_save_str)
            
            # Fall back to paramBits if h_saveParamBits not set
            if param_bits == 0:
                param_bits_db = prop_info['paramBits']
                if param_bits_db is not None:
                    param_bits_str = str(param_bits_db).strip()
                    if param_bits_str and param_bits_str != '' and param_bits_str.isdigit():
                        param_bits = int(param_bits_str)
            
            param = None
            
            if param_bits > 0:
                # Read parameter
                if pos + param_bits > len(bitstring):
                    break
                param_value_bits = bitstring[pos:pos+param_bits]
                param = self.bits_to_number_lsb(param_value_bits)
                pos += param_bits
            
            # Read value
            value_bits = prop_info['bits']
            if pos + value_bits > len(bitstring):
                break
                
            value_bits_str = bitstring[pos:pos+value_bits]
            value = self.bits_to_number_lsb(value_bits_str)
            pos += value_bits
            
            # Create PropertyInfo
            prop = PropertyInfo(prop_id, value, param, pos - value_bits - (param_bits if param else 0) - 9)
            prop.name = prop_info['name']
            prop.value_bits = value_bits
            prop.param_bits = param_bits
            prop.addv = prop_info['addv']
            
            self.properties_list.append(prop)
        
        print(f"✅ Parsed {len(self.properties_list)} properties from {filename}")
    
    def bits_to_number_lsb(self, bits: str) -> int:
        """Convert LSB-first bit string to number"""
        # Reverse to get MSB-first, then convert
        msb = bits[::-1]
        return int(msb, 2) if msb else 0
    
    def refresh_properties(self):
        """Refresh the properties treeview"""
        # Clear existing items
        for item in self.tree.get_children():
            self.tree.delete(item)
        
        # Add properties
        for prop in self.properties_list:
            display_value = prop.value - prop.addv
            
            param_str = ""
            display_info = ""
            
            if prop.param is not None:
                param_str = str(prop.param)
                
                # Look up skill/aura name
                if prop.param in self.skills_db:
                    skill_info = self.skills_db[prop.param]
                    display_info = f"{skill_info['name']} (Level {display_value})"
                else:
                    display_info = f"Param: {prop.param}, Value: {display_value}"
            else:
                display_info = f"Value: {display_value}"
            
            self.tree.insert('', tk.END, values=(
                prop.prop_id,
                prop.name,
                prop.value,
                param_str,
                display_info
            ))
    
    def on_property_select(self, event):
        """Handle property selection in treeview"""
        selection = self.tree.selection()
        if not selection:
            return
        
        item = self.tree.item(selection[0])
        values = item['values']
        
        prop_id = int(values[0])
        
        # Find the property
        selected_prop = None
        for prop in self.properties_list:
            if prop.prop_id == prop_id:
                selected_prop = prop
                break
        
        if not selected_prop:
            return
        
        # Populate form
        self.prop_id_var.set(str(selected_prop.prop_id))
        self.prop_name_var.set(selected_prop.name)
        self.value_var.set(str(selected_prop.value))
        
        display_value = selected_prop.value - selected_prop.addv
        self.display_value_var.set(str(display_value))
        
        if selected_prop.param is not None:
            self.param_var.set(str(selected_prop.param))
            
            # Look up parameter name
            if selected_prop.param in self.skills_db:
                self.param_name_var.set(self.skills_db[selected_prop.param]['name'])
            else:
                self.param_name_var.set(f"Unknown ({selected_prop.param})")
        else:
            self.param_var.set("")
            self.param_name_var.set("N/A")
        
        # Show bit structure
        self.update_bit_info(selected_prop)

    def set_selected_prop_max(self):
        """Set the value entry for the currently selected property to the maximum allowed display value."""
        try:
            prop_id = int(self.prop_id_var.get())
        except Exception:
            return

        # Find the property
        selected_prop = None
        for prop in self.properties_list:
            if prop.prop_id == prop_id:
                selected_prop = prop
                break
        if not selected_prop:
            return

        # Compute max storage and display value
        max_storage = (1 << selected_prop.value_bits) - 1
        max_display = max_storage - (selected_prop.addv if selected_prop.addv else 0)

        # Update entries
        self.value_var.set(str(max_storage))
        self.display_value_var.set(str(max_display))
    
    def update_bit_info(self, prop: PropertyInfo):
        """Update bit structure information"""
        self.bit_info_text.config(state='normal')
        self.bit_info_text.delete(1.0, tk.END)
        
        info = f"Property ID: {prop.prop_id} (9 bits)\n"
        info += f"Value Bits: {prop.value_bits} bits\n"
        
        if prop.param_bits > 0:
            info += f"Parameter Bits: {prop.param_bits} bits\n"
            info += f"Total: {9 + prop.param_bits + prop.value_bits} bits\n\n"
        else:
            info += f"Total: {9 + prop.value_bits} bits\n\n"
        
        info += f"Add Value Offset: {prop.addv}\n"
        info += f"Raw Value: {prop.value}\n"
        info += f"Display Value: {prop.value - prop.addv}\n"
        
        if prop.param is not None:
            info += f"\nParameter: {prop.param}"
            if prop.param in self.skills_db:
                info += f" ({self.skills_db[prop.param]['name']})"
        
        self.bit_info_text.insert(1.0, info)
        self.bit_info_text.config(state='disabled')
    
    def apply_changes(self):
        """Apply changes to selected property"""
        selection = self.tree.selection()
        if not selection:
            messagebox.showwarning("No Selection", "Please select a property to edit")
            return
        
        try:
            # Get values from form
            prop_id = int(self.prop_id_var.get())
            new_value = int(self.value_var.get())
            
            param_str = self.param_var.get().strip()
            new_param = int(param_str) if param_str else None
            
            # Find and update property
            for prop in self.properties_list:
                if prop.prop_id == prop_id:
                    # Validate
                    max_value = (2 ** prop.value_bits) - 1
                    if new_value > max_value:
                        messagebox.showerror("Invalid Value", 
                                           f"Value {new_value} exceeds {prop.value_bits}-bit limit (max: {max_value})")
                        return
                    
                    if new_param is not None and prop.param_bits > 0:
                        max_param = (2 ** prop.param_bits) - 1
                        if new_param > max_param:
                            messagebox.showerror("Invalid Parameter", 
                                               f"Parameter {new_param} exceeds {prop.param_bits}-bit limit (max: {max_param})")
                            return
                    
                    # Update
                    prop.value = new_value
                    if new_param is not None:
                        prop.param = new_param
                    
                    self.refresh_properties()
                    self.status_var.set(f"Updated property {prop_id}: value={new_value}, param={new_param}")
                    messagebox.showinfo("Success", "Property updated successfully!")
                    return
            
        except ValueError as e:
            messagebox.showerror("Invalid Input", f"Please enter valid numbers: {e}")
    
    def add_property(self):
        """Open dialog to add a new property"""
        AddPropertyDialog(self)
    
    def delete_property(self):
        """Delete selected property"""
        selection = self.tree.selection()
        if not selection:
            messagebox.showwarning("No Selection", "Please select a property to delete")
            return
        
        item = self.tree.item(selection[0])
        values = item['values']
        prop_id = int(values[0])
        
        result = messagebox.askyesno("Confirm Delete", 
                                     f"Delete property {prop_id} ({values[1]})?")
        
        if result:
            # Find and remove property
            for i, prop in enumerate(self.properties_list):
                if prop.prop_id == prop_id:
                    self.properties_list.pop(i)
                    self.refresh_properties()
                    self.status_var.set(f"Deleted property {prop_id}")
                    return
    
    def save_file(self):
        """Save modified D2I file"""
        if not self.current_file:
            messagebox.showwarning("No File", "Please open a file first")
            return
        
        filename = filedialog.asksaveasfilename(
            title="Save D2I File",
            filetypes=[("D2I Files", "*.d2i"), ("All Files", "*.*")],
            initialdir="d2i",
            defaultextension=".d2i"
        )
        
        if not filename:
            return
        
        try:
            # Rebuild bitstring from properties
            new_bitstring = ""
            
            for prop in self.properties_list:
                # Property ID (9 bits, LSB-first)
                prop_id_bits = number_to_binary_lsb(prop.prop_id, 9)
                new_bitstring += prop_id_bits
                
                # Parameter (if exists)
                if prop.param is not None and prop.param_bits > 0:
                    param_bits = number_to_binary_lsb(prop.param, prop.param_bits)
                    new_bitstring += param_bits
                
                # Value
                value_bits = number_to_binary_lsb(prop.value, prop.value_bits)
                new_bitstring += value_bits
            
            # Add end marker
            end_marker = '111111111'
            new_bitstring += end_marker
            
            # Pad to byte boundary
            padding_needed = (8 - (len(new_bitstring) % 8)) % 8
            new_bitstring += '0' * padding_needed
            
            # Convert to bytes
            final_bytes = bitstring_to_bytes(new_bitstring)
            
            # Write file
            with open(filename, 'wb') as f:
                f.write(final_bytes)
            
            self.status_var.set(f"Saved: {filename} ({len(final_bytes)} bytes)")
            messagebox.showinfo("Success", f"File saved successfully!\n{filename}")
            
        except Exception as e:
            messagebox.showerror("Save Error", f"Failed to save file: {e}")
            import traceback
            traceback.print_exc()
    
    def show_about(self):
        """Show about dialog"""
        messagebox.showinfo("About", 
                          "D2I Property Editor\n\n"
                          "Version 1.0\n"
                          "MedianXL Offline Tools\n\n"
                          "Edit properties in Diablo 2 item files (.d2i)\n"
                          "Supports standard and parameterized properties.")


class AddPropertyDialog:
    """Dialog for adding a new property"""
    
    def __init__(self, parent: D2IPropertyEditor):
        self.parent = parent
        
        self.dialog = tk.Toplevel(parent.root)
        self.dialog.title("Add New Property")
        self.dialog.geometry("500x400")
        self.dialog.transient(parent.root)
        self.dialog.grab_set()
        
        # Property selection
        ttk.Label(self.dialog, text="Select Property:", 
                  font=('Arial', 10, 'bold')).pack(anchor=tk.W, padx=10, pady=5)
        
        # Search frame
        search_frame = ttk.Frame(self.dialog)
        search_frame.pack(fill=tk.X, padx=10, pady=5)
        
        ttk.Label(search_frame, text="Search:").pack(side=tk.LEFT)
        self.search_var = tk.StringVar()
        self.search_var.trace('w', self.filter_properties)
        search_entry = ttk.Entry(search_frame, textvariable=self.search_var, width=30)
        search_entry.pack(side=tk.LEFT, padx=5)
        
        # Property listbox
        list_frame = ttk.Frame(self.dialog)
        list_frame.pack(fill=tk.BOTH, expand=True, padx=10, pady=5)
        
        scrollbar = ttk.Scrollbar(list_frame)
        scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
        
        self.prop_listbox = tk.Listbox(list_frame, yscrollcommand=scrollbar.set, height=10)
        scrollbar.config(command=self.prop_listbox.yview)
        self.prop_listbox.pack(fill=tk.BOTH, expand=True)
        self.prop_listbox.bind('<<ListboxSelect>>', self.on_property_select)
        
        # Populate listbox
        self.all_properties = []
        for prop_id, prop_info in sorted(parent.property_db.items()):
            display = f"{prop_id}: {prop_info['name']}"
            if prop_info['h_saveParamBits'] or prop_info['paramBits']:
                display += " [Has Parameter]"
            self.all_properties.append((prop_id, display))
            self.prop_listbox.insert(tk.END, display)
        
        # Value input
        input_frame = ttk.LabelFrame(self.dialog, text="Property Values", padding="10")
        input_frame.pack(fill=tk.X, padx=10, pady=5)
        
        ttk.Label(input_frame, text="Display Value:").grid(row=0, column=0, sticky=tk.W, pady=5)
        self.value_var = tk.StringVar(value="0")
        ttk.Entry(input_frame, textvariable=self.value_var, width=15).grid(row=0, column=1, 
                                                                           sticky=tk.W, padx=5)
        
        ttk.Label(input_frame, text="Parameter ID:").grid(row=1, column=0, sticky=tk.W, pady=5)
        self.param_var = tk.StringVar(value="")
        self.param_entry = ttk.Entry(input_frame, textvariable=self.param_var, width=15)
        self.param_entry.grid(row=1, column=1, sticky=tk.W, padx=5)
        
        self.info_label = ttk.Label(input_frame, text="Select a property to see details", 
                                    foreground="gray")
        self.info_label.grid(row=2, column=0, columnspan=2, sticky=tk.W, pady=5)
        
    # Buttons
    btn_frame = ttk.Frame(self.dialog)
    btn_frame.pack(fill=tk.X, padx=10, pady=10)

    # Max button to fill fields with their maximum allowed values
    ttk.Button(btn_frame, text="Max", command=self.set_max_value).pack(side=tk.RIGHT, padx=5)
    ttk.Button(btn_frame, text="Add", command=self.add_property).pack(side=tk.RIGHT, padx=5)
    ttk.Button(btn_frame, text="Cancel", 
           command=self.dialog.destroy).pack(side=tk.RIGHT, padx=5)
    
    def filter_properties(self, *args):
        """Filter properties based on search text"""
        search_text = self.search_var.get().lower()
        
        self.prop_listbox.delete(0, tk.END)
        for prop_id, display in self.all_properties:
            if search_text in display.lower():
                self.prop_listbox.insert(tk.END, display)
    
    def on_property_select(self, event):
        """Handle property selection"""
        selection = self.prop_listbox.curselection()
        if not selection:
            return
        
        # Get property ID from selection
        selected_text = self.prop_listbox.get(selection[0])
        prop_id = int(selected_text.split(':')[0])
        
        prop_info = self.parent.property_db[prop_id]
        # store for helpers (e.g. Max button)
        self.selected_prop_info = prop_info

        info_text = f"Bits: {prop_info['bits']}, Add: {prop_info['addv']}"
        param_bits = prop_info['h_saveParamBits'] or prop_info['paramBits'] or 0
        if param_bits > 0:
            info_text += f", ParamBits: {param_bits}"
        
        self.info_label.config(text=info_text, foreground="blue")

    def set_max_value(self):
        """Set the value/parameter fields to the maximum allowed for the selected property."""
        prop_info = getattr(self, 'selected_prop_info', None)
        if prop_info is None:
            # try to infer from current selection
            sel = self.prop_listbox.curselection()
            if not sel:
                messagebox.showwarning("No Property Selected", "Please select a property first")
                return
            selected_text = self.prop_listbox.get(sel[0])
            prop_id = int(selected_text.split(':')[0])
            prop_info = self.parent.property_db.get(prop_id)
            if prop_info is None:
                messagebox.showwarning("Unknown Property", "Could not determine property info")
                return

        try:
            bits = int(prop_info.get('bits', 0))
            addv = int(prop_info.get('addv', 0))
            raw_max = (1 << bits) - 1 if bits > 0 else 0
            display_max = raw_max - addv
            if display_max < 0:
                display_max = 0
            self.value_var.set(str(display_max))

            param_bits = prop_info.get('h_saveParamBits') or prop_info.get('paramBits') or 0
            if param_bits and int(param_bits) > 0:
                max_param = (1 << int(param_bits)) - 1
                self.param_var.set(str(max_param))
            else:
                self.param_var.set("")

        except Exception as e:
            messagebox.showerror("Error", f"Failed to compute max values: {e}")
    
    def add_property(self):
        """Add the selected property"""
        selection = self.prop_listbox.curselection()
        if not selection:
            messagebox.showwarning("No Selection", "Please select a property")
            return
        
        try:
            # Get property ID
            selected_text = self.prop_listbox.get(selection[0])
            prop_id = int(selected_text.split(':')[0])
            
            prop_info = self.parent.property_db[prop_id]
            
            # Get value
            display_value = int(self.value_var.get())
            raw_value = display_value + prop_info['addv']
            
            # Validate value
            max_value = (2 ** prop_info['bits']) - 1
            if raw_value > max_value:
                messagebox.showerror("Invalid Value", 
                                   f"Value {raw_value} exceeds {prop_info['bits']}-bit limit (max: {max_value})")
                return
            
            # Get parameter
            param = None
            param_bits = prop_info['h_saveParamBits'] or prop_info['paramBits'] or 0
            
            if param_bits > 0:
                param_str = self.param_var.get().strip()
                if param_str:
                    param = int(param_str)
                    max_param = (2 ** param_bits) - 1
                    if param > max_param:
                        messagebox.showerror("Invalid Parameter", 
                                           f"Parameter {param} exceeds {param_bits}-bit limit (max: {max_param})")
                        return
                else:
                    messagebox.showwarning("Missing Parameter", 
                                         "This property requires a parameter")
                    return
            
            # Create new property
            new_prop = PropertyInfo(prop_id, raw_value, param)
            new_prop.name = prop_info['name']
            new_prop.value_bits = prop_info['bits']
            new_prop.param_bits = param_bits
            new_prop.addv = prop_info['addv']
            
            # Add to list
            self.parent.properties_list.append(new_prop)
            self.parent.refresh_properties()
            self.parent.status_var.set(f"Added property {prop_id}: {prop_info['name']}")
            
            self.dialog.destroy()
            messagebox.showinfo("Success", "Property added successfully!")
            
        except ValueError as e:
            messagebox.showerror("Invalid Input", f"Please enter valid numbers: {e}")


def main():
    """Main entry point"""
    root = tk.Tk()
    app = D2IPropertyEditor(root)
    root.mainloop()


if __name__ == "__main__":
    main()
