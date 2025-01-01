# Cutalk
C++全栈即时通讯项目。前端基于QT实现用户交互界面，包括用户注册、密码重置、聊天界面、好友管理、聊天记录等，界面模仿微信，使用QSS美化。后端采用分布式设计，包括网关服务、验证服务、状态服务、聊天服务、Redis服务、MySQL服务，后端服务（除ChatServer）通过HTTP协议与客户端通信，不同服务之间通过gRPC协议通信，ChatServer通过TCP协议与客户端及其他ChatServer通信。

1. 用户登录界面：

   ![](https://github.com/kodaniii/_readme_images/blob/main/Cutalk/image-20241229140457299.png?raw=true)

2. 用户注册界面：

   ![](https://github.com/kodaniii/_readme_images/blob/main/Cutalk/image-20241229140321233.png?raw=true)

3. 重置密码界面

   ![](https://github.com/kodaniii/_readme_images/blob/main/Cutalk/image-20241229140433302.png?raw=true)

4. 用户聊天窗口

   ![](https://github.com/kodaniii/_readme_images/blob/main/Cutalk/image-20241229142209020.png?raw=true)

5. 用户搜索界面

   ![](https://github.com/kodaniii/_readme_images/blob/main/Cutalk/image-20241229140905720.png?raw=true)

6. 好友申请和好友标签设置界面

   ![](https://github.com/kodaniii/_readme_images/blob/main/Cutalk/image-20241229141040650.png?raw=true)

7. 好友申请列表界面

   ![](https://github.com/kodaniii/_readme_images/blob/main/Cutalk/image-20241229141231551.png?raw=true)

8. 联系人列表

   ![](https://github.com/kodaniii/_readme_images/blob/main/Cutalk/image-20241229141545820.png?raw=true)

9. 后端服务（网关服务、状态服务、聊天服务x2、验证服务）略。
