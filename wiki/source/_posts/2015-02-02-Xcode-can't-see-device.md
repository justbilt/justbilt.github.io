---
title: "Xcode无法识别设备"
categories: MacOS
date: 2015-02-02 10:33
---

感谢团队中的 [@imbahom][1] 解决这个问题

Even though this one does not address the specific problem of the OP, it might be a solution for other people finding this question.

In some circumstances, Xcode will not recognise (won't even see) a connected device that was previously recognised, even though there were no changes in Mac OS/iOS/Xcode versions. This seems to happen if you connect the device while the Mac and/or the device are locked when you connect them. The device will ask if you want to trust the computer even though you already did so, but the device will still not be visible in Xcode.

Restarting Xcode or the device do not seem to have any effect. One solution is to reboot the Mac. Another much quicker solution is to restart usbmuxd:

```
1. Quit Xcode
2. Disconnect the device
3. In a terminal window, type: `sudo pkill usbmuxd` (it will be restarted again automatically)
4. Restart Xcode
5. Connect the device
```
Your device should now be visible again in Xcode!

Hope that helps a few people.


[1]: http://www.imbahom.com/wordpress/