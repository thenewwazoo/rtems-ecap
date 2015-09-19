
#include <float.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "detector.h"

int8_t make_uniform_dist(float *p, uint8_t bins);
int8_t detector_move(float *p, uint8_t bins, float error_rate);
int8_t detector_locate(Detector *d, uint32_t latest_time);
static int8_t normalize(float *p, uint8_t bins);
int8_t calc_accel(
        float *dest, uint8_t fw_teeth_tips, 
        uint32_t t0_ticks, uint8_t t0_teeth,
        uint32_t t1_ticks, uint8_t t1_teeth);
float max_probability(float *p, uint8_t bins, uint8_t *highest_dest);
uint8_t find_tooth_dist(uint8_t *tooth_dist, uint8_t bins, uint8_t start, uint8_t end);

int8_t 
make_uniform_dist(float *p, uint8_t bins)
{
    uint8_t i = 0;

    if (bins == 0) {  /* A zero-length probability distribution is a koan. */
        return -1;
    }
    
    float recip = 1 / (float)bins;
    for (i = 0; i < bins; i++)
        p[i] = recip;

    return 0;
}

int8_t
detector_move(float *p, uint8_t bins, float error_rate)
{
    uint16_t idx;  /* uint16_t because we add 2 to bins and wrapping is bad */

    float *misses = malloc(sizeof(float)*bins);
    float *hits = malloc(sizeof(float)*bins);
    if (!misses || !hits)
        return -1;


    for (idx = 0; idx < bins; idx++) {
        misses[idx] = error_rate / 2 * p[idx];
        hits[idx] = (1 - error_rate) * p[idx];
    }

    for (idx = 2; idx < bins + 2; idx++)
        p[idx % bins]  = misses[idx%bins];
        p[idx % bins] += hits[(idx-1)%bins];
        p[idx % bins] += misses[(idx-2)%bins];

    free(misses);
    free(hits);

    return 0;
}

int8_t
detector_locate(Detector *d, uint32_t latest_time)
{
    uint8_t idx;
    float accel = 0;

    for (idx = 0; idx < d->fw_teeth_tips; idx++) {
        calc_accel(
                &accel,
                d->fw_teeth_posns,
                d->previous_timer,
                d->tooth_dist[(idx-1)<0 ? d->fw_teeth_tips-1 : idx-1],
                latest_time,
                d->tooth_dist[idx]);

        if (accel > d->max_accel)
            d->tooth_prob[idx] *= 1 - d->error_rate;  /* improbable */
        else
            d->tooth_prob[idx] *= d->error_rate;  /* probable */
    }

    if (normalize(d->tooth_prob, d->fw_teeth_tips) < 0)
        return -1;

    return 0;
}

static int8_t
normalize(float *p, uint8_t bins)
{
    float norm_coeff, sum = 0;
    uint8_t idx;

    /* because sum(P) always equals 1, normalize P */
    for (idx = 0; idx < bins; idx++)
        sum += p[idx];

    if (sum <= 0)
        return -1;

    norm_coeff = 1/sum;
    for (idx = 0; idx < bins; idx++)
        p[idx] *= norm_coeff;

    return 0;
}

int8_t
calc_accel(
        float *dest,
        uint8_t fw_teeth_tips, 
        uint32_t t0_ticks, 
        uint8_t t0_teeth,
        uint32_t t1_ticks,
        uint8_t t1_teeth)
{
    float numr, denomr;

    /* cast these now to avoid possible int overflows and ugly syntax */
    float t0 = (float)t0_ticks;
    float t1 = (float)t1_ticks;
    float d0 = (float)t0_teeth;
    float d1 = (float)t1_teeth;
    float dn = (float)fw_teeth_tips;

    if (t0 == 0.0 || t1 == 0.0 || dn == 0.0)
        return -1;

    numr = d1 * t0 - d0 * t1;
    denomr = dn * t1 * t1 * t0;

    *dest = numr / denomr;

    return 0;
}

float
max_probability(float *p, uint8_t bins, uint8_t *highest_dest)
{
    uint8_t idx;
    float max = FLT_MIN;
    for (idx = 0; idx < bins; idx++) {
        if (p[idx] > max) {
            max = p[idx];
            if (highest_dest)
                *highest_dest = idx;
        }
    }
    return max;
}

uint8_t 
find_tooth_dist(uint8_t *tooth_dist, uint8_t bins, uint8_t start, uint8_t end)
{
    uint8_t distance = 0;
    
    if (start == end || bins < 2)
        return 0;

    while (start != end) {
        start += 1;
        if (start == bins)
            start = 0;
        distance += tooth_dist[start];
    }
    return distance;
}
