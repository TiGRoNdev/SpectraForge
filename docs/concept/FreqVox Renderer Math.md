# Detailed Mathematical Framework for AFS-NVR

## 1. Sparse Voxel Encoding with Spherical Harmonics

1. Define a sparse voxel grid $V\subset\mathbb{R}^3$. Each voxel $v_i$ stores:
    - A position center $\mathbf{x}_i$.
    - Compressed radiance field via low-order Spherical Harmonic (SH) coefficients $\{c_{i,\ell,m}\}$, $\ell=0,1,2$, $m=-\ell,\dots,\ell$.
2. For any view direction $\mathbf{\omega}$, approximate incident radiance:

$$
L_i(\mathbf{\omega}) = \sum_{\ell=0}^2 \sum_{m=-\ell}^{\ell} c_{i,\ell,m} \, Y_{\ell}^{m}(\mathbf{\omega}).
$$

Here $Y_{\ell}^{m}$ are real SH basis functions.
3. Adaptive LOD via viewer position $\mathbf{x}_\text{cam}$ and gaze direction $\mathbf{d}$:

$$
\text{LOD}_i = \begin{cases}
  \text{fine}, & \|\mathbf{x}_i - \mathbf{x}_\text{cam}\| \le R_f \land \angle(\mathbf{x}_i-\mathbf{x}_\text{cam},\mathbf{d})\le \theta_f,\\
  \text{coarse}, & \text{otherwise}.
\end{cases}
$$

Choose $R_f\approx10$\,m, $\theta_f\approx10^\circ$.

## 2. Frequency-Domain Shading

1. Represent material BRDF response $M(\mathbf{\omega}_i,\mathbf{\omega}_o)$ in frequency domain via Discrete Cosine Transform (DCT):

$$
\widetilde{M}[u,v] = \sum_{p=0}^{P-1}\sum_{q=0}^{Q-1} M[p,q]\cos\Bigl(\frac{\pi u (2p+1)}{2P}\Bigr)\cos\Bigl(\frac{\pi v (2q+1)}{2Q}\Bigr).
$$
2. Incident lighting in voxel $i$ after SH evaluation yields a low-resolution “texture” $L_i[p,q]$. Compute shading as 2D convolution in frequency domain:

$$
\widetilde{S}_i[u,v] = \widetilde{L}_i[u,v]\;\odot\;\widetilde{M}[u,v],
$$

then inverse DCT:

$$
S_i[p,q] = \sum_{u=0}^{P-1}\sum_{v=0}^{Q-1} \widetilde{S}_i[u,v]\cos\Bigl(\frac{\pi u (2p+1)}{2P}\Bigr)\cos\Bigl(\frac{\pi v (2q+1)}{2Q}\Bigr).
$$
3. Complexity per voxel block: $O(PQ\log(PQ))$ via FFT acceleration when $P=Q$.

## 3. Foveated Sampling and Voxel Selection

1. Foveation weight $w_i$ based on pixel’s angular distance $\phi_i$ from gaze:

$$
w_i = \exp\Bigl(-\frac{\phi_i^2}{2\sigma^2}\Bigr),\quad \sigma\approx5^\circ.
$$
2. Adjust effective voxel count:

$$
V_{\text{eff}} = \sum_{i\in V} w_i.
$$

## 4. Temporal Reprojection \& Reconstruction

1. For frame $t$, pixel projection $\mathbf{u}_t$. Use previous frame’s color $C_{t-1}(\mathbf{u}_{t-1})$ with motion vectors $\mathbf{v}$:

$$
\mathbf{u}_{t-1} = \mathbf{u}_t - \mathbf{v}(\mathbf{u}_t).
$$
2. Blend new shading $S_t$ and history:

$$
C_t = \alpha\,S_t + (1-\alpha)\,C_{t-1}(\mathbf{u}_{t-1}),\quad \alpha=0.2.
$$
3. Only recompute shading for pixels where $\|\mathbf{v}\|>\delta$ or depth change $>\epsilon$.

## 5. Neural Enhancement Pipeline

1. Base resolution rendering at $\rho^2$ of output: $\rho\approx0.5$.
2. Tiny CNN denoiser/upscaler $f_{\theta}$:

$$
\hat{C} = f_{\theta}\bigl(\text{Upsample}(C_t)\bigr),\quad f_{\theta}:\mathbb{R}^{H\times W\times3}\to\mathbb{R}^{2H\times2W\times3}.
$$
3. Inference FLOPs estimate:

$$
\text{FLOPs} = \sum_{l=1}^{L} (K_l^2 C_{l-1} C_l H_l W_l),
$$

tuned so $\text{FLOPs}\leq1\times10^9$ to fit $\leq0.5$ ms on DSP.

## 6. Performance Constraint \& Parallel Efficiency

Total per-frame compute:

$$
T = \frac{V_{\text{eff}} \cdot O(PQ\log(PQ)) + T_{\text{invDCT}} + T_{\text{reproj}} + T_{\text{CNN}}}{E},
$$

where:

- $T_{\text{invDCT}}=O(V_{\text{eff}} PQ)$,
- $T_{\text{reproj}}\approx0.3$ ms,
- $T_{\text{CNN}}\approx0.5$ ms,
- $E\approx0.85$ (85% GPU efficiency).

Enforce $T\le3.3$ ms for 300 FPS.

***

**This mathematical framework** details all transforms (SH evaluation, DCT/IDCT, convolution in frequency domain), sampling weights, reprojection blending, and neural inference cost modeling necessary to realize **AFS-NVR** on low-segment mobile GPUs.

