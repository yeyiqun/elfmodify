# elfmodify
合并elf格式的program header pt_load


解决upx压缩so时，如果elf的program header含4个pt_load则压缩错误的问题。


修改完再执行一次 strip -R .gnu.version xx来删除无用字段更佳。
