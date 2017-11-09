rm final_labels*.txt
rm final_data*.txt

./a.out gen/spec164.gen
python parse_data.py spec164

./a.out gen/spec179.gen
python parse_data.py spec179

./a.out gen/spec181.gen
python parse_data.py spec181

./a.out gen/spec183.gen
python parse_data.py spec183

./a.out gen/spec186.gen
python parse_data.py spec186

./a.out gen/spec175.gen
python parse_data.py spec175

./a.out gen/spec188.gen
python parse_data.py spec188

./a.out gen/spec256.gen
python parse_data.py spec256

# ./a.out gen/spec300.gen
# python parse_data.py spec300

python create_datasets.py
python train.py
