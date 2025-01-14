
# MadRenderer External

This is a basic rendering library using DirectX 11. It was written to help in cheat development. 


## Features

- Text Rendering
- Image Display
- Basic Geometric Shapes
- Audio


## How to use

1. Copy the compiled library file along with the DX11.h header into your project directory.
2. Please note that when using my project, you will still need to include the other headers that my DX11.h depends on. You can simply copy the necessary includes from my pch.h file. I know this is quite annoying, but to be honest, Im lazy and likely the only one using this library at the moment xD. As soon as I have the motivation, I will improve the library so it can be included into any project without any of the additional includes that the user has to do and without the DirectXTK bullshit. (Should only take some minor changes)
4. Add DirectXTK to your project by simply following this guide: https://github.com/microsoft/DirectXTK/wiki/DirectXTK#adding-to-a-vs-solution
I recommend using project-to-project references and selecting DirectXTK_Desktop_2022_Win10.

5. Link the library to your project as you would with any other.

If you're using my custom window manager, ensure you initialize the renderer as shown below:
![image](https://github.com/user-attachments/assets/f95a2114-e14f-46bd-85b2-5f60fd903560)

Alternatively, you can use your own window manager. The DX11 class operates independently of my Window class, meaning the two can work separately:

![image](https://github.com/user-attachments/assets/84f1acfb-0bfe-427a-8c47-b8be5c39a87b)

Once set up, you’re ready to start drawing:
![image](https://github.com/user-attachments/assets/a0dce917-f2f3-4fbf-8687-2182429831b8)

Make sure to draw your content between the begin and end calls!
![image](https://github.com/user-attachments/assets/ba9b1c39-c5b4-489f-8e7d-68c99b72b1ab)

Images and Audio example
![image](https://github.com/user-attachments/assets/46647280-79ba-4e4f-8e48-260c15fa15ea)

There is a limit on how many audio files or images you can have. However, you can simply change this in the classes. Just change the max amount in the class and recompile.
Most things should be self-explanatory.

# Project Settings I used
![image](https://github.com/user-attachments/assets/73e56197-703c-4190-a7d6-031e82fa6254)
![image](https://github.com/user-attachments/assets/91c79c4a-fd20-43fc-92e9-a064f1063772)
![image](https://github.com/user-attachments/assets/17430e35-c493-42bd-bf03-2bffb00824bf)
![image](https://github.com/user-attachments/assets/5ea2a5c2-4eb4-45df-a244-ea95bcad2bd3)



