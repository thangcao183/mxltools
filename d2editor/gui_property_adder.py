#!/usr/bin/env python3
"""
Simple Tkinter GUI for adding properties to .d2i files.

Features:
- Lists properties from data/props.db (prefers header-derived display fields)
- Filter/search box for properties
- File picker to choose a .d2i file
- Value entry and Add button which calls add_property_to_file
- Inline status/log area showing operation output
"""
import sqlite3
from pathlib import Path
import threading
import io
import sys

ROOT = Path(__file__).resolve().parent
DB_PATH = ROOT / 'data' / 'props.db'


def load_properties(limit=None):
    props = []
    if not DB_PATH.exists():
        return props
    conn = sqlite3.connect(str(DB_PATH))
    cur = conn.cursor()
    cur.execute('SELECT * FROM props')
    cols = [d[0] for d in cur.description]
    for row in cur.fetchall():
        rowd = dict(zip(cols, row))
        code = rowd.get('code')
        # prefer header-derived display fields
        display = None
        for k in ('h_descStringAdd', 'h_descStringAdd_orig', 'h_descPositive', 'h_descStringAdd', 'name', 'h_stat'):
            if k in rowd and rowd[k]:
                display = rowd[k]
                break
        if not display:
            display = str(code)
        props.append((int(code), display, rowd))
        if limit and len(props) >= limit:
            break
    conn.close()
    return props


# Top-level App class removed so module can be imported without tkinter.


def main():
    # Try import tkinter here; if not available, tell the user how to install it.
    try:
        import tkinter as tk
        from tkinter import ttk, filedialog, messagebox
    except Exception:
        print('Tkinter not available in this Python environment.')
        print('On Debian/Ubuntu: sudo apt-get install python3-tk')
        print('Then run: python3 gui_property_adder.py')
        return

    # Define App now that tkinter is available in this scope
    class App(tk.Tk):
        def __init__(self):
            super().__init__()
            self.title('D2I Property Adder')
            self.geometry('800x500')

            # Top frame: file selection and value
            top = ttk.Frame(self)
            top.pack(fill='x', padx=8, pady=8)

            self.file_var = tk.StringVar()
            ttk.Label(top, text='File:').pack(side='left')
            self.file_entry = ttk.Entry(top, textvariable=self.file_var, width=60)
            self.file_entry.pack(side='left', padx=4)
            ttk.Button(top, text='Browse', command=self.browse_file).pack(side='left')

            mid = ttk.Frame(self)
            mid.pack(fill='x', padx=8, pady=4)
            ttk.Label(mid, text='Value:').pack(side='left')
            self.value_var = tk.StringVar()
            ttk.Entry(mid, textvariable=self.value_var, width=10).pack(side='left', padx=4)
            ttk.Button(mid, text='Add', command=self.on_add).pack(side='left', padx=8)
            ttk.Button(mid, text='Edit Values...', command=self.on_edit_values).pack(side='left')
            # Initialize controls
            self.auto_init_var = tk.BooleanVar(value=False)
            ttk.Checkbutton(mid, text='Auto-init if missing', variable=self.auto_init_var).pack(side='left', padx=8)
            ttk.Button(mid, text='Initialize property area', command=self.on_initialize).pack(side='left')

            # Main body: left browser, center columns area, right actions/logs
            body = ttk.Frame(self)
            body.pack(fill='both', expand=True, padx=8, pady=8)

            # Left: property browser
            left = ttk.Frame(body)
            left.pack(side='left', fill='y')
            ttk.Label(left, text='Search:').pack(anchor='w')
            self.search_var = tk.StringVar()
            self.search_var.trace_add('write', lambda *a: self.update_list())
            ttk.Entry(left, textvariable=self.search_var).pack(fill='x')
            self.listbox = tk.Listbox(left, selectmode='browse', width=40)
            self.listbox.pack(fill='y', expand=True, pady=6)

            # Center: horizontal columns area (canvas with inner frame)
            center = ttk.Frame(body)
            center.pack(side='left', fill='both', expand=True, padx=8)
            ttk.Label(center, text='Columns (properties to apply):').pack(anchor='w')

            self.canvas = tk.Canvas(center, height=300)
            self.canvas.pack(fill='both', expand=True, side='top')
            self.columns_frame = ttk.Frame(self.canvas)
            self.canvas.create_window((0, 0), window=self.columns_frame, anchor='nw')
            # horizontal scrollbar
            hsb = ttk.Scrollbar(center, orient='horizontal', command=self.canvas.xview)
            hsb.pack(fill='x', side='bottom')
            self.canvas.configure(xscrollcommand=hsb.set)

            # Right: actions and logs
            right = ttk.Frame(body)
            right.pack(side='right', fill='y')
            ttk.Label(right, text='Actions:').pack(anchor='w')
            ttk.Button(right, text='Add Selected as Column', command=self.add_selected_column).pack(fill='x', pady=4)
            ttk.Button(right, text='Info', command=self.on_info).pack(fill='x', pady=4)
            ttk.Button(right, text='Preview', command=self.preview_columns).pack(fill='x')
            ttk.Button(right, text='Apply All Columns', command=self.apply_all_columns).pack(fill='x')
            ttk.Separator(right, orient='horizontal').pack(fill='x', pady=8)
            ttk.Label(right, text='Log:').pack(anchor='w')
            self.log = tk.Text(right, width=50, height=20)
            self.log.pack(fill='both', expand=True)

            self.props = load_properties()
            self.update_list()
            self.columns = []  # list of dicts {code, display, frame, value_var}

            # update canvas scrollregion when columns_frame changes
            def on_config(event):
                self.canvas.configure(scrollregion=self.canvas.bbox('all'))
            self.columns_frame.bind('<Configure>', on_config)

        def browse_file(self):
            p = filedialog.askopenfilename(title='Select .d2i file', filetypes=[('D2I files', '*.d2i'), ('All files', '*')])
            if p:
                self.file_var.set(p)

        def update_list(self):
            q = self.search_var.get().lower()
            self.listbox.delete(0, 'end')
            for code, display, rowd in self.props:
                s = f"{code} - {display}"
                if not q or q in s.lower():
                    self.listbox.insert('end', s)

        def append_log(self, text: str):
            self.log.insert('end', text + '\n')
            self.log.see('end')

        def add_selected_column(self):
            sel = self.listbox.curselection()
            if not sel:
                messagebox.showwarning('No selection', 'Please select a property to add')
                return
            item = self.listbox.get(sel[0])
            code = int(item.split(' - ', 1)[0])
            display = item.split(' - ', 1)[1]
            # prevent duplicates
            if any(c['code'] == code for c in self.columns):
                messagebox.showinfo('Duplicate', f'Property {code} is already added')
                return

            # create a column frame
            col = ttk.Frame(self.columns_frame, relief='ridge', borderwidth=1)
            col.pack(side='left', padx=4, pady=6)
            ttk.Label(col, text=f'{code}').pack()
            ttk.Label(col, text=display, wraplength=120).pack()
            val_var = tk.StringVar()
            val_var.set(self.value_var.get())
            ttk.Entry(col, textvariable=val_var, width=10).pack(pady=4)
            def remove():
                col.destroy()
                self.columns = [c for c in self.columns if c['frame'] is not col]
            ttk.Button(col, text='Remove', command=remove).pack(pady=2)
            # move buttons
            def move_left():
                idx = next((i for i, it in enumerate(self.columns) if it['frame'] is col), None)
                if idx is None or idx == 0:
                    return
                self.columns[idx], self.columns[idx-1] = self.columns[idx-1], self.columns[idx]
                # re-pack frames
                for f in self.columns_frame.winfo_children():
                    f.pack_forget()
                for it in self.columns:
                    it['frame'].pack(side='left', padx=4, pady=6)

            def move_right():
                idx = next((i for i, it in enumerate(self.columns) if it['frame'] is col), None)
                if idx is None or idx == len(self.columns)-1:
                    return
                self.columns[idx], self.columns[idx+1] = self.columns[idx+1], self.columns[idx]
                for f in self.columns_frame.winfo_children():
                    f.pack_forget()
                for it in self.columns:
                    it['frame'].pack(side='left', padx=4, pady=6)

            btns_row = ttk.Frame(col)
            btns_row.pack()
            ttk.Button(btns_row, text='<', width=2, command=move_left).pack(side='left')
            ttk.Button(btns_row, text='>', width=2, command=move_right).pack(side='left')
            self.columns.append({'code': code, 'display': display, 'frame': col, 'value_var': val_var})

        def apply_all_columns(self):
            if not self.columns:
                messagebox.showwarning('No columns', 'Add at least one property column first')
                return
            filename = self.file_var.get()
            if not filename:
                messagebox.showwarning('No file', 'Please select a .d2i file')
                return
            props_values = []
            for c in self.columns:
                try:
                    v = int(c['value_var'].get())
                except Exception:
                    messagebox.showwarning('Invalid value', f'Invalid value for property {c["code"]}')
                    return
                props_values.append((c['code'], v))

            def bg():
                try:
                    import property_adder as pal
                except Exception as e:
                    self.append_log(f'Error importing backend: {e}')
                    return
                actual_filename = filename
                buf = io.StringIO()
                old_out, old_err = sys.stdout, sys.stderr
                sys.stdout = buf
                sys.stderr = buf
                try:
                    ok = pal.add_properties_to_file(actual_filename, props_values)
                except Exception as e:
                    buf.write(f'Exception: {e}\n')
                    ok = False
                finally:
                    sys.stdout = old_out
                    sys.stderr = old_err
                out = buf.getvalue()
                self.append_log(out)
                if ok:
                    output_file = actual_filename + '.added'
                    messagebox.showinfo('Success', f'Created {output_file}')
                else:
                    messagebox.showerror('Failed', 'Property addition failed. See log for details.')

            threading.Thread(target=bg, daemon=True).start()

        def preview_columns(self):
            if not self.columns:
                messagebox.showwarning('No columns', 'Add at least one property column first')
                return
            filename = self.file_var.get()
            if not filename:
                messagebox.showwarning('No file', 'Please select a .d2i file')
                return
            props_values = []
            for c in self.columns:
                try:
                    v = int(c['value_var'].get())
                except Exception:
                    messagebox.showwarning('Invalid value', f'Invalid value for property {c["code"]}')
                    return
                props_values.append((c['code'], v))

            try:
                import property_adder as pal
                # Load file and calculate preview
                adder = pal.PropertyAdder(filename)
                if not adder.load_file():
                    messagebox.showerror('Error', 'Failed to load file')
                    return
                
                # Calculate total bits for new properties
                total_prop_bits = 0
                for prop_id, value in props_values:
                    if prop_id in pal.PROPERTIES:
                        prop = pal.PROPERTIES[prop_id]
                        total_prop_bits += 9 + prop['bits']  # ID + value bits
                
                current_bits = len(adder.original_bitstring)
                current_bytes = len(adder.original_bytes)
                
                # Calculate final bits (content + new properties + end marker + padding)
                content_bits = adder.end_marker_pos + total_prop_bits
                total_content = content_bits + 9  # +9 for end marker
                padding = (8 - (total_content % 8)) % 8
                final_bits = total_content + padding
                final_bytes = (final_bits + 7) // 8 + 2  # +2 for JM header
                
                res = {
                    'current_bits': current_bits,
                    'final_bits': final_bits,
                    'current_bytes': current_bytes,
                    'final_bytes': final_bytes,
                    'added_bytes': final_bytes - current_bytes
                }
            except Exception as e:
                messagebox.showerror('Error', f'Preview failed: {e}')
                return
            
            dlg = tk.Toplevel(self)
            dlg.title('Preview')
            ttk.Label(dlg, text=f"Current bits: {res['current_bits']}").pack(anchor='w', padx=10, pady=2)
            ttk.Label(dlg, text=f"Final bits: {res['final_bits']}").pack(anchor='w', padx=10, pady=2)
            ttk.Label(dlg, text=f"Current bytes: {res['current_bytes']}").pack(anchor='w', padx=10, pady=2)
            ttk.Label(dlg, text=f"Final bytes: {res['final_bytes']}").pack(anchor='w', padx=10, pady=2)
            ttk.Label(dlg, text=f"Added bytes: {res['added_bytes']}").pack(anchor='w', padx=10, pady=2)
            ttk.Button(dlg, text='Close', command=dlg.destroy).pack(pady=8)

        def on_initialize(self):
            filename = self.file_var.get()
            if not filename:
                messagebox.showwarning('No file', 'Please select a .d2i file')
                return
            messagebox.showinfo('Not Implemented', 'Initialize function not available in this version.\nUse create_clean_extended tool instead.')

        def on_add(self):
            sel = self.listbox.curselection()
            if not sel:
                messagebox.showwarning('No property selected', 'Please select one or more properties from the list')
                return
            # gather selected prop ids
            prop_ids = []
            for i in sel:
                item = self.listbox.get(i)
                prop_ids.append(int(item.split(' - ', 1)[0]))
            filename = self.file_var.get()
            if not filename:
                messagebox.showwarning('No file', 'Please select a .d2i file')
                return
            try:
                value = int(self.value_var.get())
            except Exception:
                messagebox.showwarning('Invalid value', 'Please enter an integer value')
                return

            # Run add_property_to_file in a background thread and capture stdout
            def worker():
                try:
                    import property_adder as pal
                except Exception as e:
                    self.append_log(f'Error importing backend: {e}')
                    return
                buf = io.StringIO()
                old_out, old_err = sys.stdout, sys.stderr
                sys.stdout = buf
                sys.stderr = buf
                try:
                    ok = pal.add_properties_to_file(filename, [(pid, value) for pid in prop_ids])
                except Exception as e:
                    buf.write(f'Exception: {e}\n')
                    ok = False
                finally:
                    sys.stdout = old_out
                    sys.stderr = old_err

                out = buf.getvalue()
                self.append_log(out)
                if ok:
                    output_file = filename + '.added'
                    messagebox.showinfo('Success', f'Created {output_file}')
                else:
                    messagebox.showerror('Failed', 'Property addition failed. See log for details.')

            threading.Thread(target=worker, daemon=True).start()

        def on_info(self):
            filename = self.file_var.get()
            if not filename:
                messagebox.showwarning('No file', 'Please select a .d2i file')
                return
            try:
                import property_adder as pal
            except Exception as e:
                messagebox.showerror('Error', f'Cannot import backend: {e}')
                return

            # capture printed info into a buffer
            buf = io.StringIO()
            old_out, old_err = sys.stdout, sys.stderr
            sys.stdout = buf
            sys.stderr = buf
            try:
                adder = pal.PropertyAdder(filename)
                if adder.load_file():
                    print(f"File: {filename}")
                    print(f"Total bytes: {len(adder.original_bytes)}")
                    print(f"Total bits: {len(adder.original_bitstring)}")
                    print(f"End marker position: {adder.end_marker_pos}")
                    print(f"Content bits: {adder.end_marker_pos}")
                    print(f"Padding bits: {len(adder.original_bitstring) - adder.end_marker_pos - 9}")
                else:
                    print('Failed to load file')
            except Exception as e:
                print(f'Exception while getting item info: {e}')
            finally:
                sys.stdout = old_out
                sys.stderr = old_err

            out = buf.getvalue()
            # show in a dialog and append to log
            dlg = tk.Toplevel(self)
            dlg.title('Item Info')
            txt = tk.Text(dlg, wrap='none', width=120, height=30)
            txt.insert('1.0', out)
            txt.configure(state='disabled')
            txt.pack(fill='both', expand=True)
            ttk.Button(dlg, text='Close', command=dlg.destroy).pack(pady=6)
            self.append_log(out)

        def on_edit_values(self):
            sel = self.listbox.curselection()
            if not sel:
                messagebox.showwarning('No property selected', 'Please select one or more properties first')
                return
            selected = [self.listbox.get(i) for i in sel]
            # Parse (code, display)
            choices = [(int(s.split(' - ', 1)[0]), s.split(' - ', 1)[1]) for s in selected]

            # Dialog to input individual values
            dlg = tk.Toplevel(self)
            dlg.title('Edit Values')
            entries = {}

            frame = ttk.Frame(dlg)
            frame.pack(fill='both', expand=True, padx=8, pady=8)
            for cid, disp in choices:
                row = ttk.Frame(frame)
                row.pack(fill='x', pady=2)
                ttk.Label(row, text=f'{cid} - {disp}', width=40).pack(side='left')
                var = tk.StringVar()
                var.set(self.value_var.get())
                e = ttk.Entry(row, textvariable=var, width=10)
                e.pack(side='left')
                entries[cid] = var

            def on_apply():
                props_values = []
                for cid, var in entries.items():
                    try:
                        v = int(var.get())
                    except Exception:
                        messagebox.showwarning('Invalid value', f'Invalid integer for property {cid}')
                        return
                    props_values.append((cid, v))
                dlg.destroy()
                # call batch add in background
                def bg():
                    try:
                        import property_adder as pal
                    except Exception as e:
                        self.append_log(f'Error importing backend: {e}')
                        return
                    buf = io.StringIO()
                    old_out, old_err = sys.stdout, sys.stderr
                    sys.stdout = buf
                    sys.stderr = buf
                    try:
                        ok = pal.add_properties_to_file(self.file_var.get(), props_values)
                    except Exception as e:
                        buf.write(f'Exception: {e}\n')
                        ok = False
                    finally:
                        sys.stdout = old_out
                        sys.stderr = old_err
                    out = buf.getvalue()
                    self.append_log(out)
                    if ok:
                        output_file = self.file_var.get() + '.added'
                        messagebox.showinfo('Success', f'Created {output_file}')
                    else:
                        messagebox.showerror('Failed', 'Property addition failed. See log for details.')

                threading.Thread(target=bg, daemon=True).start()

            btns = ttk.Frame(dlg)
            btns.pack(fill='x', pady=8)
            ttk.Button(btns, text='Apply', command=on_apply).pack(side='right', padx=4)
            ttk.Button(btns, text='Cancel', command=dlg.destroy).pack(side='right')

    def on_edit_values(self):
        sel = self.listbox.curselection()
        if not sel:
            messagebox.showwarning('No property selected', 'Please select one or more properties first')
            return
        selected = [self.listbox.get(i) for i in sel]
        # Parse (code, display)
        choices = [(int(s.split(' - ', 1)[0]), s.split(' - ', 1)[1]) for s in selected]

        # Dialog to input individual values
        dlg = tk.Toplevel(self)
        dlg.title('Edit Values')
        entries = {}

        frame = ttk.Frame(dlg)
        frame.pack(fill='both', expand=True, padx=8, pady=8)
        for cid, disp in choices:
            row = ttk.Frame(frame)
            row.pack(fill='x', pady=2)
            ttk.Label(row, text=f'{cid} - {disp}', width=40).pack(side='left')
            var = tk.StringVar()
            var.set(self.value_var.get())
            e = ttk.Entry(row, textvariable=var, width=10)
            e.pack(side='left')
            entries[cid] = var

        def on_apply():
            props_values = []
            for cid, var in entries.items():
                try:
                    v = int(var.get())
                except Exception:
                    messagebox.showwarning('Invalid value', f'Invalid integer for property {cid}')
                    return
                props_values.append((cid, v))
            dlg.destroy()
            # call batch add in background
            def bg():
                try:
                    import property_adder as pal
                except Exception as e:
                    self.append_log(f'Error importing backend: {e}')
                    return
                buf = io.StringIO()
                old_out, old_err = sys.stdout, sys.stderr
                sys.stdout = buf
                sys.stderr = buf
                try:
                    ok = pal.add_properties_to_file(self.file_var.get(), props_values)
                except Exception as e:
                    buf.write(f'Exception: {e}\n')
                    ok = False
                finally:
                    sys.stdout = old_out
                    sys.stderr = old_err
                out = buf.getvalue()
                self.append_log(out)
                if ok:
                    messagebox.showinfo('Success', f'Created {self.file_var.get()}.added')
                else:
                    messagebox.showerror('Failed', 'Property addition failed. See log for details.')

            threading.Thread(target=bg, daemon=True).start()

        btns = ttk.Frame(dlg)
        btns.pack(fill='x', pady=8)
        ttk.Button(btns, text='Apply', command=on_apply).pack(side='right', padx=4)
        ttk.Button(btns, text='Cancel', command=dlg.destroy).pack(side='right')

    app = App()
    app.mainloop()


if __name__ == '__main__':
    main()
