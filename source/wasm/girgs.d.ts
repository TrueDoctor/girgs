// Type declarations for the girgs WebAssembly module (see bindings.cpp).
// Edge lists are flat Int32Arrays [u0, v0, u1, v1, ...] with zero based node indices.
// Positions are flat row-major Float64Arrays: node i occupies [i*dimension, (i+1)*dimension).
// A negative seed draws a random seed.

export interface GirgParams {
  n?: number;            // default 10000
  dimension?: number;    // 1..5, default 1
  ple?: number;          // power law exponent of the weights, default 2.5
  alpha?: number;        // edge probability parameter, Infinity for threshold graphs (default)
  avgDegree?: number;    // desired expected average degree, default 10
  weightSeed?: number;   // default 12
  positionSeed?: number; // default 130
  samplingSeed?: number; // default 1400
}

export interface Girg {
  weights: Float64Array;
  positions: Float64Array;
  edges: Int32Array;
  scaling: number;
}

export interface GirgsModule {
  generateWeights(n: number, ple: number, seed: number): Float64Array;
  generatePositions(n: number, dimension: number, seed: number): Float64Array;
  scaleWeights(weights: ArrayLike<number>, desiredAvgDegree: number, dimension: number, alpha: number): { weights: Float64Array; scaling: number };
  generateEdges(weights: ArrayLike<number>, positions: ArrayLike<number>, dimension: number, alpha: number, seed: number): Int32Array;
  generateGirg(params?: GirgParams): Girg;

  calculateRadius(n: number, alpha: number, T: number, avgDegree: number): number;
  calculateRadiusLikeNetworKit(n: number, alpha: number, T: number, avgDegree: number): number;
  sampleRadii(n: number, alpha: number, R: number, seed: number): Float64Array;
  sampleAngles(n: number, seed: number): Float64Array;
  sampleRadiiAndAngles(n: number, alpha: number, R: number, seed: number): { radii: Float64Array; angles: Float64Array };
  generateHyperbolicEdges(radii: ArrayLike<number>, angles: ArrayLike<number>, T: number, R: number, seed: number): Int32Array;
}

export default function createGirgs(options?: Record<string, unknown>): Promise<GirgsModule>;
