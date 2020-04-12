# CodeCraft
git版本控制参考网址https://blog.csdn.net/tina_ttl/article/details/51326684?depth_1-utm_source=distribute.pc_relevant.none-task&utm_source=distribute.pc_relevant.none-task

如果在本地文件夹创建了新文件或文件夹，需要将变化更新到.git缓存目录中，执行命令：git add 新文件(夹)名 ,然后再执行：git commit -m 'version1.0'

如果需要提交本地文件到github仓库中，执行：git push origin master （提交代码前，如果其他人访问github并更新了代码，需要先git pull origin master合并一下代码到本地，然后再提交）

查看开发日志：git log

根据标签可以回滚某个文件，执行：git reset 9671f83b12ddf411a974fb4f47ffe8fa8d93bb05(替换成要回滚的标签) main.cpp(要回滚的文件)，然后执行：git commit -m 'verson1.0'，然后git checkout main.cpp

编译：g++ -O3 main.cpp -o test -lpthread

执行：./test

git remote add origin https://github.com/你的github用户名/你的github仓库.git  
