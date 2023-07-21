
# UnityUI技术要点
- [C# 开发代码规范](#c-开发代码规范) 
---

## 涉及UI性能的技巧和原则
UGUI方面最大的性能瓶颈就是多次的drawcall，按理说在屏幕上添加一些UI这件事不应当使用超过一次的drawcall，然而实际上许多情况会导致单UI系统就会在一帧内发生多次batch。比如如果

[Unity Manual - Sprite Atlas](https://docs.unity3d.com/Manual/sprite-atlas.html)


可以看到上文中开篇第一句话，在Unity中，通常为一个纹理进行一次drawcall。这里与openGL等原始项目的渲染过程是不一样的，在那些项目中，比如要绘制一个外表面包络纹理的立方体，drawcall的数量与纹理数量是无关的

1. 降batch，比如使用图集。
2. 动静分离，使动态变化导致的UIElementDirty的范围尽量小
3. 降低overdraw，主要是避免不必要的重叠——UI组件如果发生重叠一定会发生blending
4. 关闭UI组件不需要使用的功能，比如rich text，raycast，mask，layout等
5. 不要用SetActive来隐藏组件，而是通过把组件位移到画面可视范围之外（事实上渲染时候会被裁剪）


## Unity UIToolKit EventSystem
