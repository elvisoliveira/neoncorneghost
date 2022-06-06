# neoncorneghost

Flash for pro micro-based builds

```console
qmk flash -kb crkbd/rev1/common -km neoncorneghost -bl avrdude-split-left
qmk flash -kb crkbd/rev1/common -km neoncorneghost -bl avrdude-split-right
```

Flash for Elite C or dfu bootloader builds

```console
qmk flash -kb crkbd/rev1/common -km neoncorneghost -bl dfu-split-left
qmk flash -kb crkbd/rev1/common -km neoncorneghost -bl dfu-split-right
```

These commands can be mixed if, for example, you have an Elite C on the left and a pro micro on the right.

Note: its not recommended to try to use QMK Toolbox
