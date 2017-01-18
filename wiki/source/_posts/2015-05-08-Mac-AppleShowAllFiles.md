title: "Mac 显示隐藏文件"
date: 2015-05-08 10:39:14
categories: MacOS
---


### 显示：

```
defaults write com.apple.finder AppleShowAllFiles -bool true
```

### 隐藏：

```
defaults write com.apple.finder AppleShowAllFiles -bool false
```


### 显示桌面:
```
defaults write com.apple.finder CreateDesktop -bool true && killall Finder
```

### 隐藏桌面:
```
defaults write com.apple.finder CreateDesktop -bool false && killall Finder
```