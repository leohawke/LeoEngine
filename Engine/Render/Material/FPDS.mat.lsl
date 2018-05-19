(material
	;mat文件是编辑出来的,他被读入时是一个matasset,会进行第一次求值,matasset不可变
	;当某个render操作需要用mat时,需要从matasset实例化出mat,进行第二次求值,mat运行时可变,并允许自己持有GPUbuffer
	;当某个render操作索引了某个mat,在进入渲染管道前,发生最后一次求值
	(effect ForwardPointLightDiffuseShading) ;使用那个shader
	(env math.lss) ;可以加入一个lschemesource文件中的定义到 Mat资源实例化时的 求值环境中
	(env-global math.lss) ;可以加入一个用于lazy求值时的 全局求值环境中
	(mat.albedo (float3 1 (* 1 2) (sqrt 2))) ;effect变量名(可以用[$cbuffername.]修饰) 之后是求值语句
	(metalness (lazy-oninstance (/ time 60))) ;lazy-oninstance 实例化时求值
	(worldview (lazy-onrender worldview)) ;lazy-onrender 里面的函数体会在renderer之前求值
)