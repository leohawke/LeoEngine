namespace leo
{
	//模仿Orge

	enum class MemoryCategory
	{
		/// General purpose
		MEMCATEGORY_GENERAL = 0,
		/// Geometry held in main memory
		MEMCATEGORY_GEOMETRY = 1,
		/// Animation data like tracks, bone matrices
		MEMCATEGORY_ANIMATION = 2,
		/// Nodes, control data
		MEMCATEGORY_SCENE_CONTROL = 3,
		/// Scene object instances
		MEMCATEGORY_SCENE_OBJECTS = 4,
		/// Other resources
		MEMCATEGORY_RESOURCE = 5,
		/// Scripting
		MEMCATEGORY_SCRIPTING = 6,
		/// Rendersystem structures
		MEMCATEGORY_RENDERSYS = 7,


		// sentinel value, do not use 
		MEMCATEGORY_COUNT = 8
	};

	//本质上的分配器.使用标准分配器(uint8_t)
	namespace details
	{
		using base_alloc = ::leo::aligned_alloc < std::uint8_t, 16 >;
	}
	template<MemoryCategory cate>
	using CateAlloc = details::base_alloc;

	typedef CateAlloc<MemoryCategory::MEMCATEGORY_GENERAL> GeneralAllocPolicy;
	typedef CateAlloc<MemoryCategory::MEMCATEGORY_GEOMETRY> GeometryAllocPolicy;
	typedef CateAlloc<MemoryCategory::MEMCATEGORY_ANIMATION> AnimationAllocPolicy;
	typedef CateAlloc<MemoryCategory::MEMCATEGORY_SCENE_CONTROL> SceneCtlAllocPolicy;
	typedef CateAlloc<MemoryCategory::MEMCATEGORY_SCENE_OBJECTS> SceneObjAllocPolicy;
	typedef CateAlloc<MemoryCategory::MEMCATEGORY_RESOURCE> ResourceAllocPolicy;
	typedef CateAlloc<MemoryCategory::MEMCATEGORY_SCRIPTING> ScriptingAllocPolicy;
	typedef CateAlloc<MemoryCategory::MEMCATEGORY_RENDERSYS> RenderSysAllocPolicy;


	template<typename AllocPolice>
	class AllocatedObject
	{
	private:
		static AllocPolice impl;
	public:
		explicit AllocatedObject()
		{ }
#if 1
		virtual ~AllocatedObject()
		{ }
#endif
		/// operator new, with debug line info
		void* operator new(size_t sz, const char* file, int line, const char* func)
		{
			return impl.allocate(sz);//, file, line, func);
		}

			void* operator new(size_t sz)
		{
			return impl.allocate(sz);
		}

			/// placement operator new
			void* operator new(size_t sz, void* ptr)
		{
			(void)sz;
			return ptr;
		}

			/// array operator new, with debug line info
			void* operator new[](size_t sz, const char* file, int line, const char* func)
		{
			return impl.allocate(sz);//, file, line, func);
		}

			void* operator new[](size_t sz)
		{
			return impl.allocate(sz);
		}

			void operator delete(void* ptr)
		{
			impl.deallocate(typename AllocPolice::pointer(ptr), 1);
		}

		// Corresponding operator for placement delete (second param same as the first)
		void operator delete(void* ptr, void*)
		{
			impl.deallocate(typename AllocPolice::pointer(ptr), 1);
		}

		// only called if there is an exception in corresponding 'new'
		void operator delete(void* ptr, const char* file, int line, const char* func)
		{
			impl.deallocate(typename AllocPolice::pointer(ptr), 1);// file, line, func);
		}

		void operator delete[](void* ptr)
		{
			impl.deallocate(typename AllocPolice::pointer(ptr), 1);
		}

			void operator delete[](void* ptr, const char*, int, const char*)
		{
			impl.deallocate(typename AllocPolice::pointer(ptr), 1);
		}
	};


	template<typename AllocPolice>
	class DataAllocatedObject
	{
	private:
		static AllocPolice impl;
	public:
		explicit DataAllocatedObject()
		{ }

		/// operator new, with debug line info
		void* operator new(size_t sz, const char* file, int line, const char* func)
		{
			return impl.allocate(sz);//, file, line, func);
		}

			void* operator new(size_t sz)
		{
			return impl.allocate(sz);
		}

			/// placement operator new
			void* operator new(size_t sz, void* ptr)
		{
			(void)sz;
			return ptr;
		}

			/// array operator new, with debug line info
			void* operator new[](size_t sz, const char* file, int line, const char* func)
		{
			return impl.allocate(sz);//, file, line, func);
		}

			void* operator new[](size_t sz)
		{
			return impl.allocate(sz);
		}

			void operator delete(void* ptr)
		{
			impl.deallocate(typename AllocPolice::pointer(ptr), 1);
		}

		// Corresponding operator for placement delete (second param same as the first)
		void operator delete(void* ptr, void*)
		{
			impl.deallocate(typename AllocPolice::pointer(ptr), 1);
		}

		// only called if there is an exception in corresponding 'new'
		void operator delete(void* ptr, const char* file, int line, const char* func)
		{
			impl.deallocate(typename AllocPolice::pointer(ptr), 1);// file, line, func);
		}

		void operator delete[](void* ptr)
		{
			impl.deallocate(typename AllocPolice::pointer(ptr), 1);
		}

			void operator delete[](void* ptr, const char*, int, const char*)
		{
			impl.deallocate(typename AllocPolice::pointer(ptr), 1);
		}
	};


	typedef AllocatedObject<GeneralAllocPolicy> GeneralAllocatedObject;
	typedef AllocatedObject<GeometryAllocPolicy> GeometryAllocatedObject;
	typedef AllocatedObject<AnimationAllocPolicy> AnimationAllocatedObject;
	typedef AllocatedObject<SceneCtlAllocPolicy> SceneCtlAllocatedObject;
	typedef AllocatedObject<SceneObjAllocPolicy> SceneObjAllocatedObject;
	typedef AllocatedObject<ResourceAllocPolicy> ResourceAllocatedObject;
	typedef AllocatedObject<ScriptingAllocPolicy> ScriptingAllocatedObject;
	typedef AllocatedObject<RenderSysAllocPolicy> RenderSysAllocatedObject;

	typedef ScriptingAllocatedObject    AbstractNodeAlloc;
	typedef AnimationAllocatedObject    AnimableAlloc;
	typedef AnimationAllocatedObject    AnimationAlloc;
	typedef GeneralAllocatedObject      ArchiveAlloc;
	typedef GeometryAllocatedObject     BatchedGeometryAlloc;
	typedef RenderSysAllocatedObject    BufferAlloc;
	typedef GeneralAllocatedObject      CodecAlloc;
	typedef ResourceAllocatedObject     CompositorInstAlloc;
	typedef GeneralAllocatedObject      ConfigAlloc;
	typedef GeneralAllocatedObject      ControllerAlloc;
	typedef GeometryAllocatedObject     DebugGeomAlloc;
	typedef GeneralAllocatedObject      DynLibAlloc;
	typedef GeometryAllocatedObject     EdgeDataAlloc;
	typedef GeneralAllocatedObject      FactoryAlloc;
	typedef SceneObjAllocatedObject     FXAlloc;
	typedef GeneralAllocatedObject      ImageAlloc;
	typedef GeometryAllocatedObject     IndexDataAlloc;
	typedef GeneralAllocatedObject      LogAlloc;
	typedef SceneObjAllocatedObject     MoveableAlloc;
	typedef SceneCtlAllocatedObject     NodeAlloc;
	typedef SceneObjAllocatedObject     OverlayAlloc;
	typedef RenderSysAllocatedObject    GpuParamsAlloc;
	typedef ResourceAllocatedObject     PassAlloc;
	typedef GeometryAllocatedObject     PatchAlloc;
	typedef GeneralAllocatedObject      PluginAlloc;
	typedef GeneralAllocatedObject      ProfilerAlloc;
	typedef GeometryAllocatedObject     ProgMeshAlloc;
	typedef SceneCtlAllocatedObject     RenderQueueAlloc;
	typedef RenderSysAllocatedObject    RenderSysAlloc;
	typedef GeneralAllocatedObject      RootAlloc;
	typedef ResourceAllocatedObject     ResourceAlloc;
	typedef GeneralAllocatedObject      SerializerAlloc;
	typedef SceneCtlAllocatedObject     SceneMgtAlloc;
	typedef ScriptingAllocatedObject    ScriptCompilerAlloc;
	typedef ScriptingAllocatedObject    ScriptTranslatorAlloc;
	typedef SceneCtlAllocatedObject     ShadowDataAlloc;
	typedef GeneralAllocatedObject      StreamAlloc;
	typedef SceneObjAllocatedObject     SubEntityAlloc;
	typedef ResourceAllocatedObject     SubMeshAlloc;
	typedef ResourceAllocatedObject     TechniqueAlloc;
	typedef GeneralAllocatedObject      TimerAlloc;
	typedef ResourceAllocatedObject     TextureUnitStateAlloc;
	typedef GeneralAllocatedObject      UtilityAlloc;
	typedef GeometryAllocatedObject     VertexDataAlloc;
	typedef RenderSysAllocatedObject    ViewportAlloc;
	typedef SceneCtlAllocatedObject     LodAlloc;
	typedef GeneralAllocatedObject      FileSystemLayerAlloc;
	typedef GeneralAllocatedObject      StereoDriverAlloc;
}