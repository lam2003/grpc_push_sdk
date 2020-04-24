#### 依赖git submodule
- [grpc  v1.19.x](https://github.com/grpc/grpc/tree/v1.19.x)
- [spdlog  v1.x](https://github.com/gabime/spdlog/tree/v1.x)

git递归拉取submodule代码
```
git clone https://git.yy.com/opensource/100edu-platform/push_sdk.git master 
git submodule update --init --recursive
```