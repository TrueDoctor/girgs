#include <algorithm>
#include <limits>
#include <string>
#include <utility>
#include <vector>

#include <emscripten/bind.h>
#include <emscripten/val.h>

#include <girgs/Generator.h>
#include <hypergirgs/Generator.h>

using emscripten::val;


namespace {

// Copies a C++ vector into a freshly allocated JS typed array (e.g. "Float64Array").
// The copy is required since views into wasm memory are invalidated once the heap grows.
template <typename T>
val toTypedArray(const std::vector<T>& data, const char* arrayType) {
    return val::global(arrayType).new_(emscripten::typed_memory_view(data.size(), data.data()));
}

std::vector<double> toDoubleVector(const val& array) {
    return emscripten::convertJSArrayToNumberVector<double>(array);
}

// Edge lists are returned flat as [u0, v0, u1, v1, ...]
val edgesToTypedArray(const std::vector<std::pair<int, int>>& edges) {
    std::vector<int> flat;
    flat.reserve(2 * edges.size());
    for (const auto& e : edges) {
        flat.push_back(e.first);
        flat.push_back(e.second);
    }
    return toTypedArray(flat, "Int32Array");
}

// Positions are exchanged flat in row-major order: node i occupies [i*dimension, (i+1)*dimension)
val positionsToTypedArray(const std::vector<std::vector<double>>& positions) {
    std::vector<double> flat;
    flat.reserve(positions.empty() ? 0 : positions.size() * positions.front().size());
    for (const auto& p : positions)
        flat.insert(flat.end(), p.begin(), p.end());
    return toTypedArray(flat, "Float64Array");
}

std::vector<std::vector<double>> positionsFromTypedArray(const val& array, int dimension) {
    const auto flat = toDoubleVector(array);
    const auto n = flat.size() / dimension;
    std::vector<std::vector<double>> positions(n, std::vector<double>(dimension));
    for (size_t i = 0; i < n; ++i)
        std::copy_n(flat.begin() + i * dimension, dimension, positions[i].begin());
    return positions;
}

[[noreturn]] void throwJsError(const std::string& message) {
    val::global("Error").new_(message).throw_();
}


//
// girgs
//

val generateWeights(int n, double ple, int seed) {
    return toTypedArray(girgs::generateWeights(n, ple, seed, false), "Float64Array");
}

val generatePositions(int n, int dimension, int seed) {
    return positionsToTypedArray(girgs::generatePositions(n, dimension, seed, false));
}

// Returns { weights: Float64Array, scaling: number } since JS typed arrays cannot be modified in place from here
val scaleWeights(const val& weights, double desiredAvgDegree, int dimension, double alpha) {
    auto w = toDoubleVector(weights);
    const auto scaling = girgs::scaleWeights(w, desiredAvgDegree, dimension, alpha);
    auto result = val::object();
    result.set("weights", toTypedArray(w, "Float64Array"));
    result.set("scaling", scaling);
    return result;
}

val generateEdges(const val& weights, const val& positions, int dimension, double alpha, int seed) {
    return edgesToTypedArray(girgs::generateEdges(toDoubleVector(weights), positionsFromTypedArray(positions, dimension), alpha, seed));
}

// Checks the parameters of generateGirg() before any work is done.
// Returns an empty string if the parameters are valid, otherwise a message that is thrown as a JS Error.
std::string validateGirgParams(int n, int dimension, double ple, double alpha, double avgDegree) {
    // comparisons are written so that NaN fails them
    if (!(n > 0))
        return "n must be positive";
    if (!(dimension >= 1 && dimension <= 5))
        return "dimension must be between 1 and 5";
    if (!(ple > 2.0))
        return "ple must be greater than 2";
    // scaleWeights supports alpha in (0,1), (1,inf) and inf
    if (!(alpha > 0.0) || alpha == 1.0)
        return "alpha must be positive and not equal to 1";
    if (!(avgDegree > 0.0 && avgDegree < n - 1))
        return "avgDegree must be in (0, n-1)";
    return "";
}

// Convenience entry point running the full pipeline:
// generateGirg({ n, dimension, ple, alpha, avgDegree, weightSeed, positionSeed, samplingSeed })
//   -> { weights, positions, edges, scaling }
val generateGirg(const val& params) {
    auto get = [&](const char* key, double fallback) {
        const auto v = params[key];
        return v.isUndefined() ? fallback : v.as<double>();
    };

    const auto n = static_cast<int>(get("n", 10000));
    const auto dimension = static_cast<int>(get("dimension", 1));
    const auto ple = get("ple", 2.5);
    const auto alpha = get("alpha", std::numeric_limits<double>::infinity());
    const auto avgDegree = get("avgDegree", 10);
    const auto weightSeed = static_cast<int>(get("weightSeed", 12));
    const auto positionSeed = static_cast<int>(get("positionSeed", 130));
    const auto samplingSeed = static_cast<int>(get("samplingSeed", 1400));

    const auto error = validateGirgParams(n, dimension, ple, alpha, avgDegree);
    if (!error.empty())
        throwJsError(error);

    auto weights = girgs::generateWeights(n, ple, weightSeed, false);
    const auto positions = girgs::generatePositions(n, dimension, positionSeed, false);
    const auto scaling = girgs::scaleWeights(weights, avgDegree, dimension, alpha);
    const auto edges = girgs::generateEdges(weights, positions, alpha, samplingSeed);

    auto result = val::object();
    result.set("weights", toTypedArray(weights, "Float64Array"));
    result.set("positions", positionsToTypedArray(positions));
    result.set("edges", edgesToTypedArray(edges));
    result.set("scaling", scaling);
    return result;
}


//
// hypergirgs
//

// Returns { radii: Float64Array, angles: Float64Array }
val sampleRadiiAndAngles(int n, double alpha, double R, int seed) {
    const auto sample = hypergirgs::sampleRadiiAndAngles(n, alpha, R, seed, false);
    auto result = val::object();
    result.set("radii", toTypedArray(sample.first, "Float64Array"));
    result.set("angles", toTypedArray(sample.second, "Float64Array"));
    return result;
}

val generateHyperbolicEdges(const val& radii, const val& angles, double T, double R, int seed) {
    auto r = toDoubleVector(radii);
    auto a = toDoubleVector(angles);
    return edgesToTypedArray(hypergirgs::generateEdges(r, a, T, R, seed));
}

} // namespace


EMSCRIPTEN_BINDINGS(girgs) {
    using namespace emscripten;

    function("generateWeights", &generateWeights);
    function("generatePositions", &generatePositions);
    function("scaleWeights", &scaleWeights);
    function("generateEdges", &generateEdges);
    function("generateGirg", &generateGirg);

    function("calculateRadius", &hypergirgs::calculateRadius);
    function("calculateRadiusLikeNetworKit", &hypergirgs::calculateRadiusLikeNetworKit);
    function("sampleRadii", +[](int n, double alpha, double R, int seed) {
        return toTypedArray(hypergirgs::sampleRadii(n, alpha, R, seed, false), "Float64Array");
    });
    function("sampleAngles", +[](int n, int seed) {
        return toTypedArray(hypergirgs::sampleAngles(n, seed, false), "Float64Array");
    });
    function("sampleRadiiAndAngles", &sampleRadiiAndAngles);
    function("generateHyperbolicEdges", &generateHyperbolicEdges);
}
