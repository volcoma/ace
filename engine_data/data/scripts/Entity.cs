using System;
using System.Runtime.CompilerServices;

namespace Ace
{
namespace Core
{

struct Entity : IEquatable<Entity>
{
	public readonly uint Id;
	
	public static Entity Null => new Entity(0);
	
	internal Entity(uint id)
	{
		Id = id;
	}
	
	public override bool Equals(object obj)
    {
        if (obj == null || !(obj is Entity))
            return false;

        return Equals((Entity)obj);
    }

	public bool Equals(Entity other)
	{
		if (ReferenceEquals(this, other))
			return true;

		return Id == other.Id;
	}
	
	public static bool operator ==(Entity lhs, Entity rhs) => lhs.Equals(rhs);

	public static bool operator !=(Entity lhs, Entity rhs) => !(lhs == rhs);
	
	public override int GetHashCode() => (Id).GetHashCode();
	
}
}
}


