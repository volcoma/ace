using System;
using System.Runtime.CompilerServices;

namespace Ace
{
namespace Core
{
public class Scene
{

	public Scene()
	{
		Internal_CreateScene();
		
		var e = Entity.Null;
		e = CreateEntity("Test");
	}

	~Scene()
	{
		Internal_DestroyScene();
	}
	
	public static Entity CreateEntity(string tag = "Unnamed")
	{
		unsafe { return new Entity(Internal_CreateEntity(tag)); }
	}

	public static void DestroyEntity(Entity entity)
	{
		unsafe
		{
			if (!Internal_IsEntityValid(entity.Id))
				return;

			Internal_DestroyEntity(entity.Id);
		}
	}
	

	[MethodImpl(MethodImplOptions.InternalCall)] 
	private extern void Internal_CreateScene();

	[MethodImpl(MethodImplOptions.InternalCall)] 
	private extern void Internal_DestroyScene();
	
	[MethodImpl(MethodImplOptions.InternalCall)] 
	private static extern uint Internal_CreateEntity(string tag);
	
	[MethodImpl(MethodImplOptions.InternalCall)] 
	private static extern bool Internal_DestroyEntity(uint id);

	[MethodImpl(MethodImplOptions.InternalCall)] 
	private static extern bool Internal_IsEntityValid(uint id);
}

}
}


