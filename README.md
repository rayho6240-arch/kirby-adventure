# README

## Student Information

* Student ID(s): E24146644, E24146709
* Name(s): 何家睿, 張翔富
* Last Update: 2026/06/03

---

## Environment

* Operating System: Windows 10 / Windows 11
* Language: C++
* Framework: Qt 5.12.12
* Compiler: MinGW 64-bit
* IDE: Qt Creator

---

## How to Run

There are two ways to run this project:

### **method 1.**  Extract `demo.zip` and run `KirbyAdventure.exe`

download `demo.zip` at follow link:  
src:   https://drive.google.com/file/d/1SmYTbCPTWD3dlUVE0T1Q8O7VVebhYmN6/view?usp=sharing

1. UNZIP `demo.zip` to any folder.
2. Open the folder.
3. Locate the game executable `KirbyAdventure.exe` (or a similarly named `.exe`).
4. Double-click the executable to start the game.     



### **method 2.** Open `game/kirby-adventure.pro` in Qt Creator and build/run

1. Open Qt Creator.
2. Select `File` > `Open File or Project...`.
3. Navigate to the `game` folder and select `kirby-adventure.pro`.
4. Choose the appropriate Qt kit, such as `Qt 5.12.12 MinGW 64-bit`.
5. Click `Configure Project`.
6. Click the `Build` button or press `Ctrl+B` to build.
7. After the build completes, click `Run` or press `Ctrl+R` to launch the game.




### **!!** Important Resource Placement (for method 2) **!!**

The following folders/files must be placed in the **same directory as the executable (`.exe`)**:

src: https://drive.google.com/drive/folders/1ppwK49KR3e4hzd5XtsGcKVZdFC6MQC67?usp=sharing

* UNZIP three files, then import to the folder same as .exe
* Transition cutscene videos
* Ending videos
* Background music resources

If these files are missing or placed in the wrong location:
* The game will crash  


```text
root/
├── build_environment/          --> set up by Qt Creator IDE
│   ├── *.exe
│   ├── bg_music/               --> downloaded from Google Drive
│   ├── stage_transition/       --> downloaded from Google Drive
│   └── finish_animation/       --> downloaded from Google Drive
│
└── game/
    └── *.pro
```

---

## Controls

| Key          | Function                                       |
| ------------ | ---------------------------------------------- |
| ← / →        | Move                                           |
| Z            | Jump                                           |
| X            | Inhale / Attack                                |
| ↑            | Enter portal / Fly                             |
| ↓            | Crouch                                         |
| Q            | Open Skill Menu                                |
| ← / → (Menu) | Select ability                                 |
| Enter        | Confirm selected ability                       |
| V            | Drop current ability and return to Normal form |

---

## Notes / Known Issues

* Please avoid pressing any key immediately after launching the game.
  During the initial scene loading phase, rapid input may occasionally cause unexpected behavior.

* If the game becomes stuck or unresponsive, restarting the game usually resolves the issue.

* Some cutscene and ending resources are loaded dynamically from external files.
  Make sure all media folders remain in the correct directory.



---

## Dependencies

* Qt 5.12.12
* Qt Multimedia module
* Required image set / audio  resource files
