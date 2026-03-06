# DataQueryUsingOpenMP

## Description

This is a C++ project that highlights the use of OpenMP to perform threaded querying of data loaded into memory. This is a mini project for supporting research as part of CMPE-275 at San Jose State University.

---
### Dataset

Downloading and Preparing the TLC HVFHV Dataset

This project uses the NYC Taxi & Limousine Commission High Volume For-Hire Vehicle (HVFHV) trip record dataset.

The TLC publishes monthly datasets in Apache Parquet format. For benchmarking and experimentation, we combine January–May 2023 into a single CSV file using DuckDB.

Official dataset source:

https://www.nyc.gov/site/tlc/about/tlc-trip-record-data.page

#### 1. Create a Working Directory
```
mkdir -p ~/hvfhv_2023_jan_may
cd ~/hvfhv_2023_jan_may
```

#### 2. Download Monthly Parquet Files
Download the first five months of 2023 HVFHV trip data.
```
curl -O https://d37ci6vzurychx.cloudfront.net/trip-data/fhvhv_tripdata_2023-01.parquet
curl -O https://d37ci6vzurychx.cloudfront.net/trip-data/fhvhv_tripdata_2023-02.parquet
curl -O https://d37ci6vzurychx.cloudfront.net/trip-data/fhvhv_tripdata_2023-03.parquet
curl -O https://d37ci6vzurychx.cloudfront.net/trip-data/fhvhv_tripdata_2023-04.parquet
curl -O https://d37ci6vzurychx.cloudfront.net/trip-data/fhvhv_tripdata_2023-05.parquet
```
Verify the files were downloaded:
```
ls -lh *.parquet
```
Example output:
```
fhvhv_tripdata_2023-01.parquet
fhvhv_tripdata_2023-02.parquet
fhvhv_tripdata_2023-03.parquet
fhvhv_tripdata_2023-04.parquet
fhvhv_tripdata_2023-05.parquet
```

#### 3. Install DuckDB

DuckDB is used to read the parquet files and export them as CSV.

macOS
```
brew install duckdb
```
Linux
```
sudo apt install duckdb
```
Verify installation
```
duckdb --version
```

#### 4. Verify Row Count (Optional)

Before exporting the dataset to CSV, verify the total number of rows.
```
duckdb -c "
SELECT COUNT(*) AS row_count
FROM read_parquet('fhvhv_tripdata_2023-0[1-5].parquet');
"
```
Expected result: ~230 million rows (The exact number may vary slightly depending on dataset revisions.)

#### 5. Combine Parquet Files into a Single CSV

DuckDB can read multiple parquet files using glob patterns.
```
duckdb -c "
COPY (
SELECT *
FROM read_parquet('fhvhv_tripdata_2023-0[1-5].parquet')
)
TO 'fhvhv_2023_jan_may.csv'
(FORMAT CSV, HEADER);
"
```
This creates:
```
fhvhv_2023_jan_may.csv
```

#### 6. Verify the CSV
```
ls -lh fhvhv_2023_jan_may.csv
head -n 5 fhvhv_2023_jan_may.csv
wc -l fhvhv_2023_jan_may.csv
```
Example preview:
```
hvfhs_license_num,dispatching_base_num,originating_base_num,...
HV0003,B03404,B03404,2023-01-01 00:18:06,...
```

#### 7. Optional: Create a Filtered Dataset

For faster experimentation, you may wish to filter the dataset.

Example: only trips with trip_miles ≥ 4
```
duckdb -c "
COPY (
SELECT *
FROM read_parquet('fhvhv_tripdata_2023-0[1-5].parquet')
WHERE trip_miles >= 4
)
TO 'fhvhv_2023_miles_ge_4.csv'
(FORMAT CSV, HEADER);
"
```

#### 8. Disk Space Expectations

Approximate sizes:

Format	Size
Parquet (5 months)	~20–25 GB
Combined CSV	~80–100 GB

CSV is significantly larger because it is not compressed.

#### 9. Notes
- The dataset contains high-volume ride-hailing trips (Uber/Lyft).
- Each row represents a single completed trip.
- CSV format is used because it simplifies parsing in C/C++ benchmarking implementations.
- Parquet remains preferable for analytics systems.

#### 10. Citation

NYC Taxi & Limousine Commission (TLC). TLC Trip Record Data.

https://www.nyc.gov/site/tlc/about/tlc-trip-record-data.page
