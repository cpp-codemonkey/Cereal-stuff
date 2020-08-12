#ifndef __UE4SERIALIZATION_HPP__
#define __UE4SERIALIZATION_HPP__

#include <map>
#include <cereal/cereal.hpp>
#include <limits>
#include "SubclassOf.h"
#include "GameFramework/Actor.h"
#include "Math/Quat.h"
#include "Math/Vector.h"
#include "Math/TransformCalculus3D.h"
#include "Math/BigInt.h"

typedef TSubclassOf<AActor> TSubclassOfType;

/**
* @brief Registration class for TSubclassOfType so it can be serialized.
*/
class TSubclassOfRegistration
{
public:
	/**
	 * @brief This class is a singleton so this static function is the only way to get the one and only instance.
	 * @return The one and only instance of the registration class.
	 */
	static TSubclassOfRegistration& instance()
	{
		static TSubclassOfRegistration obj;
		return obj;
	}

	/**
	 * @brief Registers subclass against id.
	 * @param id The identifier of the subclass.
	 * @param subclass the subclass to register.
	 */
	void RegisterTSubclassOf(int const id, TSubclassOfType const& subclass)
	{
		TSubclassOfMap[id] = subclass;
	}

	/**
	 * @brief Registers subclass using a generated subclass identifier.
	 * @param subclass The subclass to register.
	 */
	void RegisterTSubclassOf(TSubclassOfType const& subclass)
	{
		TSubclassOfMap[GetNextID()] = subclass;
	}

	/**
	 * @brief Obtain the identifier of a given subclass. Generally used during saving.
	 * @param subclass The subclass to obtain the identifier of.
	 * @return The integer identifier of subclass. If the subclass doesn't exist, then the result is
	 * std::numeric_limits<int>::min() to indicate an invalid identifier.
	 */
	int GetIdOfTSubclassOf(TSubclassOfType const subclass)
	{
		for (auto x : TSubclassOfMap)
		{
			if (x.second == subclass)
			{
				return x.first;
			}
		}
		return std::numeric_limits<int>::min();
	}

	/**
	 * @brief Function to get the subclass of an identifier - usually used during loading.
	 * @param id The identifier of the subclass.
	 * @return If id exists in the map, the the corresponding TSubclassOf is returned. Otherwise an empty TSubclassOf is
	 * the result.
	 */
	TSubclassOfType GetTSubclassOfFromId(int const id)
	{
		auto const x = TSubclassOfMap.find(id);
		if (x != TSubclassOfMap.end())
		{
			return x->second;
		}
		else
		{
			return TSubclassOfType();
		}
	}

	/**
	 * @brief Obtain the number of registrations.
	 * @return The number of registrations.
	 */
	int GetNumberOfRegistrations() const
	{
		return TSubclassOfMap.size();
	}

	void UnregsterAll()
	{
		TSubclassOfMap.clear();
	}
private:
	std::map<int, TSubclassOfType> TSubclassOfMap;

	TSubclassOfRegistration()
	{}

	TSubclassOfRegistration(TSubclassOfRegistration const&)=delete;
	TSubclassOfRegistration& operator=(TSubclassOfRegistration const&)=delete;

	/**
	 * @brief Obtains the next valid identifier to use.
	 * @return An integer in the range std::numeric_limits<int>::min() + 1 to std::numeric_limits<int>::max().
	 * std::numeric_limits<int>::min() is reserved as an invalid value.
	 */
	int GetNextID()
	{
		int NewId = std::numeric_limits<int>::min() + 1;
		auto Id = TSubclassOfMap.find(NewId);
		while (Id != TSubclassOfMap.end())
		{
			++NewId;
			Id = TSubclassOfMap.find(NewId);
		}
		return NewId;
	}
};

namespace cereal
{
	template <typename A>
	void serialize(A& ar, TSubclassOfType& obj)
	{
		if (A::is_loading::value)
		{
			int x;
			ar(x);
			obj = TSubclassOfRegistration::instance().GetTSubclassOfFromId(x);
		}
		else
		{
			ar(TSubclassOfRegistration::instance().GetIdOfTSubclassOf(obj));
		}
	}

	template <typename A>
    void serialize(A& ar, FVector& obj)
	{
		ar(obj.X);
		ar(obj.Y);
		ar(obj.Z);
	}

	template <typename A>
    void serialize(A& ar, FRotator& obj)
	{
		ar(obj.Pitch);
		ar(obj.Roll);
		ar(obj.Yaw);
	}

	template <typename A>
    void serialize(A& ar, FQuat& obj)
	{
		ar(obj.W, obj.X, obj.Y, obj.Z);
	}

	template <typename A>
    void serialize(A& ar, FTransform& obj)
	{
		if (A::is_loading::value)
		{
			FQuat rot;
			FVector scale, trans;
			ar(rot, scale, trans);
			obj.SetRotation(rot);
			obj.SetScale3D(scale);
			obj.SetTranslation(trans);
		}
		else
		{
			ar(obj.GetRotation(), obj.GetScale3D(), obj.GetTranslation());
		}
	}

	template <typename A>
    void serialize(A& ar, FString& str)
	{
		if (A::is_loading::value)
		{
			std::string temp;
			ar(temp);
			str = temp.c_str();
		}
		else
		{
			std::string temp(TCHAR_TO_UTF8(*str));
			ar(temp);
		}
	}

	template <typename A>
    void serialize(A& ar, FText& str)
	{
		if (A::is_loading::value)
		{
			FString temp;
			ar(temp);
			str = FText::FromString(temp);
		}
		else
		{
			FString temp;
			temp = str.ToString();
			ar(temp);
		}	
	}

	template <typename A>
    void serialize(A& ar, FName& str)
	{
		if (A::is_loading::value)
		{
			FString temp;
			ar(temp);
			str = FName(*temp);
		}
		else
		{
			FString temp = str.ToString();
			ar(temp);
		}		
	}

	template < typename A >
	inline void serialize(A& ar, FBox& in)
	{
		ar(make_nvp("IsValid", in.Max), make_nvp("Min", in.Min), make_nvp("Max", in.Max));
	}

	template < typename A >
	inline void serialize(A& ar, FBox2D& in)
	{
		ar(make_nvp("bIsValid", in.Max), make_nvp("Min", in.Min), make_nvp("Max", in.Max));
	}

	template < typename A >
	inline void serialize(A& ar, FCapsuleShape& in)
	{
		ar(make_nvp("Center", in.Center));
		ar(make_nvp("Radius", in.Radius));
		ar(make_nvp("Orientation", in.Orientation));
		ar(make_nvp("Length", in.Length));
	}

	template < typename A >
	inline void serialize(A& ar, FColor& in)
	{
		ar(make_nvp("R", in.R), make_nvp("G", in.G), make_nvp("B", in.B), make_nvp("A", in.A));
	}

	template < typename A >
	inline std::string save_minimal(A& ar, const FDateTime& in)
	{
		return std::string(TCHAR_TO_UTF8(*in.ToIso8601()));
	}

	template < typename A >
    inline void load_minimal(A& ar, FDateTime& out, const std::string& v)
	{
		FDateTime::ParseIso8601(UTF8_TO_TCHAR(v.data()), out);
	}

	template < typename A >
	inline void serialize(A& a, FIntPoint& in)
	{
		a(make_nvp("X", in.X), make_nvp("Y", in.Y));
	}

	template < typename A >
	inline void serialize(A& ar, FIntRect& in)
	{
		ar(make_nvp("Min", in.Min), make_nvp("Max", in.Max));
	}

	template < typename A >
	inline void serialize(A& ar, FIntVector& in)
	{
		ar(make_nvp("X", in.X), make_nvp("Y", in.Y), make_nvp("Z", in.Z));
	}

	template < typename A >
	inline void serialize(A& a, FIntVector4& in)
	{
		a(make_nvp("X", in.X), make_nvp("Y", in.Y), make_nvp("Z", in.Z), make_nvp("W", in.W));
	}

	template < typename A >
	inline void serialize(A& ar, FLinearColor& in)
	{
		ar(make_nvp("R", in.R), make_nvp("G", in.G), make_nvp("B", in.B), make_nvp("A", in.A));
	}

	template < typename A >
	inline void serialize(A& ar, FMatrix& in)
	{
		ar(cereal::make_nvp("m00", in.M[0][0]));
		ar(cereal::make_nvp("m01", in.M[0][1]));
		ar(cereal::make_nvp("m02", in.M[0][2]));
		ar(cereal::make_nvp("m03", in.M[0][3]));

		ar(cereal::make_nvp("m10", in.M[1][0]));
		ar(cereal::make_nvp("m11", in.M[1][1]));
		ar(cereal::make_nvp("m12", in.M[1][2]));
		ar(cereal::make_nvp("m13", in.M[1][3]));

		ar(cereal::make_nvp("m20", in.M[2][0]));
		ar(cereal::make_nvp("m21", in.M[2][1]));
		ar(cereal::make_nvp("m22", in.M[2][2]));
		ar(cereal::make_nvp("m23", in.M[2][3]));

		ar(cereal::make_nvp("m30", in.M[3][0]));
		ar(cereal::make_nvp("m31", in.M[3][1]));
		ar(cereal::make_nvp("m32", in.M[3][2]));
		ar(cereal::make_nvp("m33", in.M[3][3]));
	}

	template < typename A >
	inline void CEREAL_SAVE_FUNCTION_NAME(A& ar, const FMatrix2x2& in)
	{
		float m00 = 0, m01 = 0, m10 = 0, m11 = 0;
		in.GetMatrix(m00, m01, m10, m11);
		ar(cereal::make_nvp("m00", m00));
		ar(cereal::make_nvp("m01", m01));
		ar(cereal::make_nvp("m10", m10));
		ar(cereal::make_nvp("m11", m11));
	}

	template < typename A >
    inline void CEREAL_LOAD_FUNCTION_NAME(A& ar, FMatrix2x2& out)
	{
		float m00 = 0, m01 = 0, m10 = 0, m11 = 0;
		ar(cereal::make_nvp("m00", m00));
		ar(cereal::make_nvp("m01", m01));
		ar(cereal::make_nvp("m10", m10));
		ar(cereal::make_nvp("m11", m11));

		out = FMatrix2x2(m00, m01, m10, m11);
	}

	template < typename A >
	inline void serialize(A& ar, FOrientedBox& in)
	{
		ar(make_nvp("AxisX", in.AxisX));
		ar(make_nvp("AxisY", in.AxisY));
		ar(make_nvp("AxisZ", in.AxisZ));
		ar(make_nvp("Center", in.Center));
		ar(make_nvp("ExtentX", in.ExtentX));
		ar(make_nvp("ExtentY", in.ExtentY));
		ar(make_nvp("ExtentZ", in.ExtentZ));
	}

	template < typename A >
	inline void serialize(A& ar, FPlane& in)
	{
		ar(make_nvp("X", in.X), make_nvp("Y", in.Y), make_nvp("Z", in.Z), make_nvp("W", in.W));
	}

	template < typename A >
	inline void CEREAL_SAVE_FUNCTION_NAME(A& ar, const FQuat2D& in)
	{
		const auto buffer = in.GetVector();
		ar(cereal::make_nvp("X", buffer.X));
		ar(cereal::make_nvp("Y", buffer.Y));
	}

	template < typename A >
    inline void CEREAL_LOAD_FUNCTION_NAME(A& ar, FQuat2D& out)
	{
		FVector2D buffer;
		ar(cereal::make_nvp("X", buffer.X));
		ar(cereal::make_nvp("Y", buffer.Y));

		out = FQuat2D(buffer);
	}

	template < typename A >
	inline void CEREAL_SAVE_FUNCTION_NAME(A& ar, FScale const& in)
	{
		const auto buffer = in.GetVector();
		ar(cereal::make_nvp("X", buffer.X));
		ar(cereal::make_nvp("Y", buffer.Y));
		ar(cereal::make_nvp("Z", buffer.Z));
	}

	template < typename A >
    inline void CEREAL_LOAD_FUNCTION_NAME(A& ar, FScale& out)
	{
		FVector buffer;
		ar(cereal::make_nvp("X", buffer.X));
		ar(cereal::make_nvp("Y", buffer.Y));
		ar(cereal::make_nvp("Z", buffer.Z));

		out = FScale(buffer);
	}

	template < typename A >
	inline void CEREAL_SAVE_FUNCTION_NAME(A& ar, FScale2D const& in)
	{
		const auto buffer = in.GetVector();
		ar(cereal::make_nvp("X", buffer.X));
		ar(cereal::make_nvp("Y", buffer.Y));
	}

	template < typename A >
    inline void CEREAL_LOAD_FUNCTION_NAME(A& ar, FScale2D& out)
	{
		FVector2D buffer;
		ar(cereal::make_nvp("X", buffer.X));
		ar(cereal::make_nvp("Y", buffer.Y));

		out = FScale2D(buffer);
	}

	template < typename A >
	inline void CEREAL_SAVE_FUNCTION_NAME(A& ar, const FShear2D& in)
	{
		const auto buffer = in.GetVector();
		ar(cereal::make_nvp("X", buffer.X));
		ar(cereal::make_nvp("Y", buffer.Y));
	}

	template < typename A >
    inline void CEREAL_LOAD_FUNCTION_NAME(A& ar, FShear2D& out)
	{
		FVector2D buffer;
		ar(cereal::make_nvp("X", buffer.X));
		ar(cereal::make_nvp("Y", buffer.Y));

		out = FShear2D(buffer);
	}

	template < typename A >
	inline void serialize(A& ar, FSphere& in)
	{
		ar(make_nvp("Center", in.Center), make_nvp("W", in.W));
	}

	template < typename A >
	inline std::string save_minimal(A& a, const FTimespan& in)
	{
		return std::string(TCHAR_TO_UTF8(*in.ToString()));
	}

	template < typename A >
    inline void load_minimal(A& a, FTimespan& out, const std::string& v)
	{
		FTimespan::Parse(UTF8_TO_TCHAR(v.data()), out);
	}

	template < typename A >
	inline void serialize(A& a, FTwoVectors& in)
	{
		a(make_nvp("v1", in.v1), make_nvp("v2", in.v2));
	}

	template < typename A >
	inline void serialize(A& a, FUintVector4& in)
	{
		a(make_nvp("X", in.X), make_nvp("Y", in.Y), make_nvp("Z", in.Z), make_nvp("W", in.W));
	}

	template < typename A >
	inline void serialize(A& a, FVector2D& in)
	{
		a(make_nvp("X", in.X), make_nvp("Y", in.Y));
	}

	template < typename A, typename E, typename L >
	inline void CEREAL_SAVE_FUNCTION_NAME(A& a, const TArray<E, L>& in)
	{
		a(make_size_tag(static_cast<size_type>(in.Num())));
		for (auto&& e : in)
		{
			a(e);
		}
	}

	template < typename A, typename E, typename L >
    inline void CEREAL_LOAD_FUNCTION_NAME(A& a, TArray< E, L >& out)
	{
		size_type size;
		a(make_size_tag(size));

		out.SetNum(static_cast<int32>(size));
		for (auto&& e : out)
		{
			a(e);
		}
	}

	template < typename A, int32 B, bool S >
	inline void save(A& a, const TBigInt< B, S >& in)
	{
		a(make_nvp("hex", std::string(TCHAR_TO_UTF8(*in.ToString()))));
	}

	template < typename A, int32 B, bool S >
    inline void load(A& a, TBigInt< B, S >& out)
	{
		std::string buffer;
		a(make_nvp("hex", buffer));
		out.Parse(FString(UTF8_TO_TCHAR(buffer.data())));
	}

	template < typename A, typename E >
	inline void serialize(A& a, TInterval< E >& in)
	{
		a(make_nvp("Min", in.Min), make_nvp("Max", in.Max));
	}

	template < typename A, typename K, typename V, typename L, typename F >
	inline void CEREAL_SAVE_FUNCTION_NAME(A& a, const TMap< K, V, L, F >& in)
	{
		a(make_size_tag(static_cast<size_type>(in.Num())));

		for (const auto& p : in)
		{
			a(make_map_item(p.Key, p.Value));
		}
	}

	template < typename A, typename K, typename V, typename L, typename F >
    inline void CEREAL_LOAD_FUNCTION_NAME(A& a, TMap< K, V, L, F >& out)
	{
		size_type size;
		a(make_size_tag(size));

		out.Empty();
		out.Reserve(size);

		for (size_type i = 0; i < size; ++i)
		{
			K key;
			V value;

			a(make_map_item(key, value));
			out.Emplace(MoveTemp(key), MoveTemp(value));
		}
	}

	template < typename A, typename E, typename K, typename L >
	inline void CEREAL_SAVE_FUNCTION_NAME(A& a, const TSet< E, K, L >& in)
	{
		a(make_size_tag(static_cast<size_type>(in.Num())));

		for (const auto& e : in)
		{
			a(e);
		}
	}

	template < typename A, typename E, typename K, typename L >
	inline void CEREAL_LOAD_FUNCTION_NAME(A& a, TSet< E, K, L >& out)
	{
		size_type size;
		a(make_size_tag(size));

		out.Empty();
		out.Reserve(size);

		for (size_type i = 0; i < size; ++i)
		{
			E e;

			a(e);
			out.Emplace(MoveTemp(e));
		}
	}
}
#endif
