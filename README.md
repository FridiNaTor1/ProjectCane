<img width="1919" height="1029" alt="Screenshot 2026-06-08 140318" src="https://github.com/user-attachments/assets/ea5af858-ed42-49d4-b18b-cbc0ca772c9b" />
<img width="1919" height="1029" alt="Screenshot 2026-06-08 135833" src="https://github.com/user-attachments/assets/9d14a2b3-360a-4da5-8574-07e0a42e7511" />
<img width="1919" height="1029" alt="Screenshot 2026-06-08 135925" src="https://github.com/user-attachments/assets/8b8e737b-4283-429b-924f-47235f696829" />
<img width="1919" height="1029" alt="Screenshot 2026-06-08 135942" src="https://github.com/user-attachments/assets/198ce439-b7fc-423b-acab-14dee85ccf04" />
<img width="1919" height="1034" alt="Screenshot 2026-06-08 140000" src="https://github.com/user-attachments/assets/f43787b3-8a96-4e46-bf36-5be2287c8e7f" />
<img width="1919" height="1027" alt="Screenshot 2026-06-08 140043" src="https://github.com/user-attachments/assets/f1b00742-cbc1-4ef8-9485-fe2a15877fa8" />
<img width="1919" height="1027" alt="Screenshot 2026-06-08 140026" src="https://github.com/user-attachments/assets/56d6e3e4-db35-4bc3-879f-b41c119340ba" />




![Logo 300](https://github.com/user-attachments/assets/49b1b7d4-87db-4134-8f52-dbef28a79d54)



# Sly Trilogy Map Viewer Project

## 📖 Project Description

Simple map viewer for sly 1 right now proto and release version

> 💻 **Note:** Only **Windows** is currently supported. Other platforms will be considered once reasonable progress is made.

---

## ⚙️ Building the Project

To compile the project, use the instructions below based on your desired target:

### Linux Retail Viewer
1. Install a C++17 compiler, CMake, OpenGL development files, and GLFW development files. If GLFW is not installed, CMake will try to fetch and build GLFW 3.4 automatically.
2. Configure and build:
   ```sh
   cmake -S . -B build
   cmake --build build -j
   ```
3. Run the viewer:
   ```sh
   ./build/projectcane
   ```
4. Use **File > Open ISO** to select a retail NTSC Sly 1 ISO. Extracted maps are cached under `~/.cache/projectcane`, then available from **File > Open Extracted Map**.
   You can also verify/extract from the terminal with:
   ```sh
   ./build/projectcane --extract-iso "/path/to/Sly Cooper and the Thievius Raccoonus (USA).iso"
   ```

### 🧪 May 19, 2002 Prototype
1. Place the following shader files in the same directory as the executable (where the project will be built):
   - `blot.vert`
   - `blot.frag`
   - `glob.vert`
   - `glob.frag`
   - `screen.vert`
   - `screen.frag`
2. In **Solution Explorer**, right-click on `Sly1-Proto`
3. Select **Set as Startup Project**
4. Click **Build** → **Run**

### 📦 Retail Release
1. Use the [Sly 1 File Extractor](https://github.com/theclub654/Sly-1-File-Extractor) to extract level files from the NTSC ISO.
2. Place the following shader files in the same directory as the executable (where the project will be built):
   - `blot.vert`
   - `blot.frag`
   - `glob.vert`
   - `glob.frag`
   - `geom.vert`
   - `geom.frag`
   - `dysh.vert`
   - `dysh.frag`
   - `screen.vert`
   - `screen.frag`
3. In **Solution Explorer**, right-click on `Sly1-Release`
4. Select **Set as Startup Project**
5. Click **Build** → **Run**

---
