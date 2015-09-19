
typedef 
struct Detector {
    uint8_t current_tooth;
    bool has_sync;
    float sync_threshold;    /* if we have more than sync_threshold confidence, we are synced      */

    uint32_t previous_timer;
    float max_accel;

    uint8_t *tooth_dist;     /* Pointer to array containing tooth distances, e.g. { 2, 1, 1 }      */
    uint8_t fw_teeth_tips;   /* Number of actual teeth on the flywheel (e.g. 59 for a 60-1 wheel)  */
    uint8_t fw_teeth_posns;  /* Number of places where a tooth could be (e.g. 60 for a 60-1 wheel) */

    float *tooth_prob;       /* Pointer to an array containing the prior probability distribution  */
    float confidence;        /* Our highest confidence, i.e. max(tooth_prob)                       */
    float error_rate;        /* We don't calculate error_rate dynamically - mostly because if we   */
                             /*  track it in real-time, it can get low enough that we never get    */
                             /*  sync again - oops - so we use an experimentally determined value. */
} Detector;

int8_t make_uniform_dist(float *p, uint8_t bins);
int8_t detector_move(float *p, uint8_t bins, float error_rate);
int8_t detector_locate(Detector *d, uint32_t latest_time);
float max_probability(float *p, uint8_t bins, uint8_t *highest_dest);
int8_t calc_accel(
        float *dest, uint8_t fw_teeth_tips, 
        uint32_t t0_ticks, uint8_t t0_teeth,
        uint32_t t1_ticks, uint8_t t1_teeth);
uint8_t find_tooth_dist(uint8_t *tooth_dist, uint8_t bins, uint8_t start, uint8_t end);
