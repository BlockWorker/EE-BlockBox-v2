import math
import sys
import matplotlib.pyplot as plt
import numpy as np
import scipy
import scipy.stats as st

chgcounts = [47800, 48000, 48300, 48600, 48800, 48970]
chgvolts = [16.23, 16.29, 16.40, 16.54, 16.62, 16.69]
dsgcounts_ul = [48500, 47600, 47400, 47100, 46600, 45800, 45700]
dsgvolts_ul = [16.58, 16.31, 16.25, 16.14, 15.97, 15.70, 15.64]
dsgcounts_l = [48700, 47800, 47600, 47200, 46700, 45900, 45500]
dsgvolts_l = [16.50, 16.24, 16.19, 16.07, 15.90, 15.63, 15.55]

chg_sl, chg_int, _, _, _ = st.linregress(chgcounts, chgvolts)
line_x = [dsgcounts_l[-1], chgcounts[-1]]
chgline_y = [chg_int + x * chg_sl for x in line_x]

dsg_ul_sl, dsg_ul_int, _, _ ,_ = st.linregress(dsgcounts_ul, dsgvolts_ul)
dsgline_ul_y = [dsg_ul_int + x * dsg_ul_sl for x in line_x]

dsg_l_sl, dsg_l_int, _, _ ,_ = st.linregress(dsgcounts_l, dsgvolts_l)
dsgline_l_y = [dsg_l_int + x * dsg_l_sl for x in line_x]

print("Charge slope:", chg_sl)
print("Inverse of chg slope:", 1.0 / chg_sl)
print("Charge intercept:", chg_int)

print("Discharge low load slope:", dsg_ul_sl)
print("Inverse of dsg ll slope:", 1.0 / dsg_ul_sl)
print("Discharge low load intercept:", dsg_ul_int)

print("Discharge mid load slope:", dsg_l_sl)
print("Inverse of dsg ml slope:", 1.0 / dsg_l_sl)
print("Discharge mid load intercept:", dsg_l_int)

plt.scatter(chgcounts, chgvolts, label="Charge measurements")
plt.scatter(dsgcounts_ul, dsgvolts_ul, label="Discharge measurements, low load")
plt.scatter(dsgcounts_l, dsgvolts_l, label="Discharge measurements, mid load")
plt.plot(line_x, chgline_y, label="Charge regression")
plt.plot(line_x, dsgline_ul_y, label="Discharge regression, low load")
plt.plot(line_x, dsgline_l_y, label="Discharge regression, mid load")
plt.legend()
plt.show()
