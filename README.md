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

1. Open the project using Qt Creator.
2. Build the project using the Qt 5.12.12 MinGW 64-bit kit.
3. Execute the generated `.exe` file.

### Important Resource Placement

The following folders/files must be placed in the **same directory as the executable (`.exe`)**:

* Transition cutscene videos
* Ending videos
* Background music / sound effect resources

If these files are missing or placed in the wrong location:

* cutscenes may not play correctly
* background music may fail to load
* ending animations may not appear

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

* The Skill Menu only allows selecting abilities that have already been unlocked during gameplay.

---

## Dependencies

* Qt 5.12.12
* Qt Multimedia module
* Required image / audio / video resource files
