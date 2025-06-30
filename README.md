# ResolutionTweak

This simple tool allows you to quickly switch between two resolutions—native and custom—with a single click.

It eliminates the need to navigate through the Windows settings to change the resolution.

When launched, the tool automatically switches between your native and custom resolution—switching to the custom resolution if you're on the native one, and vice versa.

You can adjust the resolution using a configuration (cfg) file.

```
CustomRes
CX=1920
CY=1440
CR=165

NativeRes
NX=2560
NY=1440
NR=165
```

> [!NOTE]
Make sure that if you use a custom resolution that is not already available in Windows, you add it to the Custom Resolutions section in the NVIDIA Control Panel or AMD Adrenalin Software.

# Why?

Idk.. I created this to quickly switch to 4:3 when playing CS. This allows me to play 4:3 in fullscreen windowed mode, where I don't need to worry about the long alt+tabbing issue that occurs in fullscreen mode and often bugs out when playing 4:3.
