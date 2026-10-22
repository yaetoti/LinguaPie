# LinguaPie - Radial Language Switcher

<img width="1919" height="1040" alt="image" src="https://github.com/user-attachments/assets/38ccb6d8-7284-41ab-b732-ac81078b595e" />

A helpful tool I developed because pressing Win + Space thousands times was too slow and exhausting, and Windows 11 macros for quick switch have limited choise of key combinations.

**Features**
- Hold Mouse_5 to show overlay. Move the cursor to the corresponding segment and release to switch language system-wide
- Currently selected language is highlighted
- Selector background is drawn on a single triangle
- Works on multi-monitor setup

**Limitations (TODO)**
- Activation button is hardcoded at the moment. I need to add JSON settings file -> add GUI -> investigate how to make it work on different device configurations
- Language names are hardcoded. There is a registry that stores full layout names in default system locale. The downsite is that they are too long, so some customization mechanism will come in handy.
- There's a REALLY rare bug when mouse input stops working and it's probably related to deadlock in the low-level mouse hook. No idea how to reproduce it
- Text alignment works poorly. No clipping, no font resizing
- No public release for now because I'm being a lazy ass. So you have to build it manually

### Demo

https://github.com/user-attachments/assets/46914e64-2a48-4ae8-b525-9408f9d70004

