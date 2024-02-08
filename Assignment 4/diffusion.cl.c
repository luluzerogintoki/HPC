__kernel
void
computeIter(
    __global float *currentBox,
    __global float *nextBox,
    __const int width,
    __const int height,
    __const float diffusion
    )
{
    //updated as h(i,j) + c * ( (h(i-1,j) + h(i+1,j) + h(i,j-1) + h(i,j+1))/4 - h(i,j) )
    //where c is a diffusion constant. 
    //We consider the boundary has constant temperature 0.

    int ix = get_global_id(0);
    int jx = get_global_id(1);

    int index = ix*width + jx;
    float left = (jx-1>=0) ? currentBox[index-1] : 0.;
    float right = (jx+1<width) ? currentBox[index+1] : 0.;
    float up = (ix-1>=0) ? currentBox[index-width] : 0.;
    float down = (ix+1<height) ? currentBox[index+width] : 0.;
    nextBox[index] = currentBox[index] + diffusion * (0.25 * (left+right+up+down) - currentBox[index]);
}

__kernel
void
reduction(
  __global float *box,
  __local float *scratch,
  __const int sz,
  __global float *result
  )
{
  int gsz = get_global_size(0);
  int gix = get_global_id(0);
  int lsz = get_local_size(0);
  int lix = get_local_id(0);

  float acc = 0;
  for ( int cix = get_global_id(0); cix < sz; cix += gsz )
    acc += box[cix];

  scratch[lix] = acc;
  barrier(CLK_LOCAL_MEM_FENCE);

  for(int offset = lsz/2; offset > 0; offset /= 2) {
    if ( lix < offset )
      scratch[lix] += scratch[lix+offset];
    barrier(CLK_LOCAL_MEM_FENCE);
  }

  if ( lix == 0 )
    result[get_group_id(0)] = scratch[0];
}

__kernel
void
computeAbsDiff(
    __global float *box,
    __const float average,
    __const int height
    )
{

    int ix = get_global_id(0);
    int jx = get_global_id(1);

    int index = ix*height + jx;
    box[index] -= average;
    box[index] = (box[index]>=0) ? box[index] : box[index] * (-1.0);
}