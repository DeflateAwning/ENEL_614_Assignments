import polars as pl
import serial
import serial.tools.list_ports # important
import easygui
from loguru import logger
import re
import time
import sys
import hvplot

# pip install polars pyserial loguru easygui hvplot

def prompt_for_serial_port():
	port_list = serial.tools.list_ports.comports()
	port_list_str: list[str] = [port.device for port in port_list]
	logger.info(f"Available ports: {port_list_str}")

	if not port_list:
		logger.error("No serial ports found. Exiting...")
		sys.exit(1)

	if len(port_list_str) == 1:
		logger.info(f"Only one port found: {port_list_str[0]}")
		return port_list_str[0]

	port = easygui.choicebox("Select the serial port", choices=[port.device for port in port_list])

	if not port:
		logger.error("No port selected. Exiting...")
		sys.exit(1)

	return port

def read_serial_data(port: str) -> pl.DataFrame:
	"""Reads data from the serial port and returns a DataFrame with the data.
	Columns: timestamp, cap_value
	"""
	logger.info(f"Starting reading data. Press Ctrl+C to stop...")

	with serial.Serial(port, 9600, timeout=1) as ser:
		data: list[dict] = []
		start_sampling_time = time.time()
		last_print_msg_time = time.time()

		try:
			while True:
				line = ser.readline()
				line = line.decode("utf-8", errors='ignore').strip()

				cap_value_search = re.search(r"REPORT_CAP_pF=(\d+)", line)

				if cap_value_search:
					cap_value = int(cap_value_search.group(1))
					data.append({
						"timestamp": time.time() - start_sampling_time,
						"cap_value": cap_value,
					})

					if time.time() - last_print_msg_time > 0.5:
						logger.info(f"Read {len(data)} samples so far. Last value: {cap_value:>8} pF (at {time.time()-start_sampling_time:.1f} sec)...")
						last_print_msg_time = time.time()
				else:
					logger.debug(f"Received: {line}")

		except KeyboardInterrupt:
			logger.info("Got keyboard interrupt. Exiting...")

	df = pl.DataFrame(data)
	df = df.with_columns(
		pl.col('cap_value').replace({
			(0xFFFFFFFF - 6): 0, # replace the over-range indicator with 0 to avoid skewing the axis limits
		})
	)
	df = df.with_columns(
		cap_value_log = pl.col('cap_value').log10(),
	)
	return df

def main():
	serial_version = serial.__version__
	logger.info(f"Using pyserial version: {serial_version}")

	port = prompt_for_serial_port()
	logger.info(f"Selected port: {port}")

	df = read_serial_data(port)

	logger.info(f"Done reading cap data: {df}")
	
	# Plot the data
	plt1 = df.plot.line(
		x='timestamp', y='cap_value_log', title='log_10(Capacitance) vs. Time',
		ylabel='log_10(Capacitance Value) [log_10(F)]', xlabel='Time (s)',
		# ylim=(0, 1023),
	)
	plt2 = df.plot.line(
		x='timestamp', y='cap_value', title='Capacitance (pF) vs. Time',
		ylabel='Capacitance Value (pF)', xlabel='Time (s)',
		# ylim=(0, 3.3),
	)
	
	logger.info(f"Showing plotted data.")
	hvplot.show(plt1 + plt2)

	logger.info(f"All done.")

if __name__ == "__main__":
	main()
