
当我在一个已经配置了git的文件夹内创建一个新的文件，比如此文件githubUse.md，其立刻被Vscode标记为一个绿色的U，即Untracked未追踪。我打开终端，输入：
```
git add githubUse.md
```
其从绿色U 变成了橙色的M，即Modified已修改。

现在如果直接进行git push想把已经新建并修改的项目发到github是不行的。这是因为我们的git记忆的commit仍然是上一次的commit，commit中并没有任何的更改。此时如果直接git push,会：

```
PS C:\Users\tail\Desktop\游戏开发笔记本> git push -u origin master
Everything up-to-date
Branch 'master' set up to track remote branch 'master' from 'origin'.
```
而前往github发现内容没有任何更改。

我们要将修改更新成一个新的提交：
```
PS C:\Users\tail\Desktop\游戏开发笔记本> git commit -m "IwanttoCommit"
[master 4cba0c7] IwanttoCommit
 1 file changed, 2 insertions(+)
 create mode 100644 githubUse.md
```
那么我们这时再进行push。如果此时报错，很有可能是因为远端的仓库已经被其他人更新过，而本地尚未同步。如果不存在这种情况(我不知道我这样的哪来的fork)
