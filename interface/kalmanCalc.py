import numpy as np
import re
import matplotlib.pyplot as plt

class KalmanFilter1D:
   
    def __init__(self, x0, P0, Q, R):
        
        self.x = x0  # State estimate
        self.P = P0  # Error covariance
        self.Q = Q   # Process noise
        self.R = R   # Measurement noise

    def update(self, z):
        x_pred = self.x
        P_pred = self.P + self.Q

        # --- Update step ---
        K = P_pred / (P_pred + self.R)  # Kalman Gain
        self.x = x_pred + K * (z - x_pred)  # New state estimate
        self.P = (1.0 - K) * P_pred  # New error covariance
        
        return self.x

def rssi_to_distance(rssi, A=-61.0, n=2.5):
    return 10 ** ((A - rssi) / (10 * n))

def simulate_rssi_from_distance(distance, A=-61.0, n=2.0, noise_std=0.0):
   
    if distance <= 0:
        return A + np.random.normal(0, noise_std)

    
    rssi = A - 10 * n * np.log10(distance)
    
    
    noisy_rssi = rssi + np.random.normal(0, noise_std)
    
    return noisy_rssi

# --- Main Script ---
if __name__ == "__main__":
    
    
    f = open("rssi.txt", "r")
    file_content = f.read()
    
    distances = [float(num) for num in re.findall(r'[\d.]+', file_content)]
    plt.plot(distances)

    
    Q = 0.1   # Process noise covariance
    R = 9.0   # Measurement noise covariance (variance of noise_std=3.0)
    
    
    initial_noisy_rssi = simulate_rssi_from_distance(distances[0])
    P0 = 10.0 # Initial error covariance
    kalman_filter = KalmanFilter1D(x0=initial_noisy_rssi, P0=P0, Q=Q, R=R)

    
    distancesKALMAN = []
    
 
    for true_distance in distances:
       
        noisy_rssi = simulate_rssi_from_distance(true_distance)
        
        filtered_rssi = kalman_filter.update(noisy_rssi)
        
        
        estimated_distance = rssi_to_distance(filtered_rssi)
        distancesKALMAN.append(estimated_distance)
        #print(f"{true_distance:<15.2f} | {noisy_rssi:<15.2f} | {filtered_rssi:<15.2f} | {estimated_distance:<15.2f}")
    #plt.plot(distancesKALMAN)
    plt.figure(figsize=(12, 8))
    
    # Original vs Kalman-filtered distances
    plt.subplot(2, 2, 1)
    plt.plot(distances, label='Original Distances')
    plt.plot(distancesKALMAN, label='Kalman Filtered')
    plt.title('Distance Measurements Comparison')
    plt.xlabel('Measurement Index')
    plt.ylabel('Distance (m)')
    plt.legend()
    plt.grid(True)
    
    # Histogram of Kalman-filtered distances
    plt.subplot(2, 2, 2)
    plt.hist(distancesKALMAN, bins=20, color='skyblue', edgecolor='black')
    plt.title('Histogram of Kalman Filtered Distances')
    plt.xlabel('Distance (m)')
    plt.ylabel('Frequency')
    plt.grid(True)
    
    # Boxplot of Kalman-filtered distances
    plt.subplot(2, 2, 3)
    plt.boxplot(distancesKALMAN, vert=True, patch_artist=True)
    plt.title('Boxplot of Kalman Filtered Distances')
    plt.ylabel('Distance (m)')
    plt.grid(True)
    
    plt.tight_layout()
    plt.show()
    
    with open('floats.txt', 'w') as f:
    	f.write(" ".join(map(str,  distancesKALMAN)))
           
