/// @ref gtx_optimum_pow
/// @file glm/gtx/optimum_pow.inl

namespace glm
{
	template<typename genType>
	GLM_FUNC_QUALIFIER genType pow2(genType const& x)
	{
		return x * x;
	}

	template<typename genType>
	GLM_FUNC_QUALIFIER genType pow3(genType const& x)
	{
		return x * x * x;
	}

	template<typename genType>
	GLM_FUNC_QUALIFIER genType pow4(genType const& x)
	{
		return (x * x) * (x * x);
	}
}//namespace glm
// CG_REVISION f0d732361be3e2b0495d47edcd0f0acf37b5c1f4
