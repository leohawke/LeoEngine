(material
	;mat�ļ��Ǳ༭������,��������ʱ��һ��matasset,����е�һ����ֵ,matasset���ɱ�
	;��ĳ��render������Ҫ��matʱ,��Ҫ��matassetʵ������mat,���еڶ�����ֵ,mat����ʱ�ɱ�,�������Լ�����GPUbuffer
	;��ĳ��render����������ĳ��mat,�ڽ�����Ⱦ�ܵ�ǰ,�������һ����ֵ
	(effect ForwardPointLightDiffuseShading) ;ʹ���Ǹ�shader
	(env math.lss) ;���Լ���һ��lschemesource�ļ��еĶ��嵽 Mat��Դʵ����ʱ�� ��ֵ������
	(env-global math.lss) ;���Լ���һ������lazy��ֵʱ�� ȫ����ֵ������
	(mat.albedo (float3 1 (* 1 2) (sqrt 2))) ;effect������(������[$cbuffername.]����) ֮������ֵ���
	(metalness (lazy-oninstance (/ time 60))) ;lazy-oninstance ʵ����ʱ��ֵ
	(worldview (lazy-onrender worldview)) ;lazy-onrender ����ĺ��������renderer֮ǰ��ֵ
)