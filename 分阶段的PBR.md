# 内容简介
本文主要基于以下资料:
[learn openGL对PBR的介绍](https://learnopengl-cn.github.io/07%20PBR/01%20Theory/)
[闫令琪老师Games202网课对于PBR的介绍](https://www.bilibili.com/video/BV1YK4y1T7yY?p=5&vd_source=be2d1feef9acea76535db5dd5a391e1a)

资料上讲的更清楚、更好，因此强烈建议看资料

# 辐射度量学知识
辐射通量：辐射通量Φ的单位是瓦特，即单位时间内光源射出的能量。
凡是光，均是不同波长的单色光的加总。对于每种单色光的能量函数进行积分，就能得到辐射通量。如图，这张图的x是单色光波长，y是单色光能量，积分得到的面积就是单位时间光的能量，也就是光源的辐射通量

![](./markdown_pic/opengl-16.jpg)
波长介于390nm到700nm（纳米）的光被认为是处于可见光光谱中，也就是说它们是人眼可见的波长，因此我们只计算这部分单色光。

辐射强度：辐射强度(Radiant Intensity)表示的是在单位球面上，一个光源向每单位立体角所投送的辐射通量。举例来说，假设一个全向光源向所有方向均匀的辐射能量，辐射强度就能帮我们计算出它在一个单位面积（立体角）内的能量大小：

辐射率 Radience，用L表示：
![](./markdown_pic/opengl-17.jpg)

# PBR基础理论
![](./markdown_pic/opengl-20.png)
![](./markdown_pic/opengl-19.png)


最终的渲染方程算式：
![](./markdown_pic/opengl-18.jpg)

# PBR 直接光照实践
直接光照作为切入点是因为，直接光对于渲染方程做了一个超越维度的简化：我们可以直接把关于wi的积分脱去。
试想一下，假设在一个直接点光源场景下，实际上我们并不用对所有立体角求渲染方程各项并叠加——因为只有一个方向的wi是有光的，那就是lightDir方向。这样一来渲染方程直接退化成：
![](./markdown_pic/opengl-21.png)
可谓恐怖如斯。
我们可以有效地将其辐射强度建模为其辐射通量: 一个常量向量`(23.47,21.31,20.79)`

# IBL 基于图像的光照实践
注：预计算在IBL当中的语义是非单一的。202中介绍的roughness-costheta纹理是一种预计算(事实上是对于镜面反射积分项进行的预计算)，而对于image的输入进行一个卷积处理也是预计算，乃至于漫反射项用黎曼求和

LOG的IBL的工作流可以切分成这样几个步骤：
step1 获取.hdr的辐照度的球面贴图，一般**等距柱状投影图Equirectangular Map**
step2 从等距柱状投影生成cubemap
step3 环境立方体贴图的卷积，生成辐照度cubemap

.hdr的辐照度球面贴图是这样的：
![](./markdown_pic/opengl-22.jpg)
这种Equirectangular Map,其形态是矩形的，而信息却是球状的，且内容组织与常规的纹理有差异。在使用`stb_image.h`后，我们可以直接把这种贴图读取成我们习惯上的2D纹理。

辐照度贴图的意义在于，
![](./markdown_pic/pbr-1.jpg)
