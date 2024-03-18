# Lvgl on UEFI Environment.

## Demo

```
build -p LvglPkg\LvglPkg.dsc -m LvglPkg\LvglApp\LvglApp.inf -a X64 -t VS2015x86 -b RELEASE
```

![LvglApp](./Demo/Images/Demo.png)

### Demo Download

[LvglApp.efi](./Demo/Bin/LvglApp.efi)

### Demo Usage

1. Boot to UEFI Shell
2. LvglApp.efi
3. `Esc` to exit

## TODO
- [ ] Mouse
- [ ] Demo Selection
- [ ] ...