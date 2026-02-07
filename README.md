# Secure Shredder Pro 🛡️

**Secure Shredder Pro** is a lightweight, high-performance C++ utility designed to permanently destroy sensitive data. It ensures that files are unrecoverable by even advanced forensic software.



## 🔍 The Problem
When you delete a file in Windows, the data stays on your disk. The OS simply marks that space as "available." Forensic tools can easily "undelete" these files. 

## 🚀 The Solution
Secure Shredder Pro solves this by:
1. **Bitwise Overwriting:** Replaces file contents with random junk data.
2. **Metadata Ghosting:** Wipes the file's "fingerprint" (timestamps and size) from the system.
3. **Safety Blacklist:** Prevents accidental deletion of critical `C:\Windows` or `System32` files.

<img width="967" height="430" alt="image" src="https://github.com/user-attachments/assets/7a0200e7-8a83-4759-95bb-bd6fa7372ca2" />

## ✨ Key Features
* **Forensic Metadata Reset:** Automatically resets file creation/access/write dates to **1980-01-01** before destruction.
* **Integrated UX:** No separate installer needed. Double-click the app to manage Right-Click menu integration.
* **Multi-Pass Logic:** * `Quick Mode`: 1-pass overwrite (Optimized for SSD longevity).
  * `Deep Mode`: 3-pass forensic-grade overwrite (DoD 5220.22-M inspired).
* **Process Persistence:** Uses `SetThreadExecutionState` to prevent Windows from sleeping during heavy shredding tasks.
* **Smart Progress:** Real-time ANSI-colored progress bars and audio cues upon completion.

## 🛠️ Installation & Usage
1. **Download** the `SecureShredder.exe`.
2. **Setup:** Double-click the `.exe` and select **Option 1** to integrate it into your Windows Context Menu.
3. **Shred:** * **Right-Click:** Select any file/folder and choose `Secure Shred File`.
   * **Drag & Drop:** Drag files directly onto the `SecureShredder.exe` icon to process them in bulk.

## 🏗️ Technical Details
* **Language:** C++17
* **Platform:** Windows (Win32 API)
* **Compiler Flags:** `-O3` (High optimization), `-s` (Strip symbols), `-lkernel32 -luser32 -ladvapi32 -lshell32`
* **Subsystem:** Console-based with ANSI escape code support.

## ⚠️ Disclaimer
**Use with caution.** Data destroyed with this tool cannot be recovered by any known means. The author is not responsible for accidental data loss.
