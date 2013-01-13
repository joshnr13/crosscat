#include <iostream>

#include "Cluster.h"
#include "utils.h"
#include "numerics.h"
#include "View.h"
#include "RandomNumberGenerator.h"

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>

typedef boost::numeric::ublas::matrix<double> matrixD;
using namespace std;
typedef vector<Cluster*> vectorCp;
typedef set<Cluster*> setCp;
typedef map<int, Cluster*> mapICp;
typedef setCp::iterator setCp_it;
typedef mapICp::iterator mapICp_it;
typedef vector<int>::iterator vectorI_it;

void print_cluster_memberships(View& v) {
  cout << "Cluster memberships" << endl;
  setCp_it it = v.clusters.begin();
  for(; it!=v.clusters.end(); it++) {
    Cluster &cd = **it;
    cout << cd.get_row_indices() << endl;
  }
  cout << "num clusters: " << v.get_num_clusters() << endl;
}

void insert_and_print(View& v, map<int, vector<double> > data_map,
		      int cluster_idx, int row_idx) {
  vector<double> row = data_map[row_idx];
  Cluster& cluster = v.get_cluster(cluster_idx);
  v.insert_row(row, cluster, row_idx);
  cout << "v.insert_row(" << row << ", " << cluster_idx << ", " \
	    << row_idx << ")" << endl;
  cout << "v.get_score(): " << v.get_score() << endl;
}

void print_with_header(View &v, string header) {
  cout << endl;
  cout << "=================================" << endl;
  cout << header << endl;
  v.print();
  cout << header << endl;
  cout << "=================================" << endl;
  cout << endl;
}

void remove_all_data(View &v, map<int, vector<double> > data_map) {
  vector<int> rows_in_view;
  for(mapICp_it it=v.cluster_lookup.begin(); it!=v.cluster_lookup.end(); it++) {
    rows_in_view.push_back(it->first);
  }
  for(vectorI_it it=rows_in_view.begin(); it!=rows_in_view.end(); it++) {
    int idx_to_remove = *it;
    vector<double> row = data_map[idx_to_remove];
    vector<int> global_indices(row.size(),1);
    global_indices[0] = 0;
    std::partial_sum(global_indices.begin(), global_indices.end(), global_indices.begin());
    vector<double> aligned_row = v.align_data(row, global_indices);
    cout << "aligned_row: " << aligned_row << endl;
    v.remove_row(aligned_row, idx_to_remove);
  }
  cout << "removed all data" << endl;
  v.print();
  //
  for(setCp_it it=v.clusters.begin(); it!=v.clusters.end(); it++) {
    v.remove_if_empty(**it);
  }
  assert(v.get_num_vectors()==0);
  assert(v.get_num_clusters()==0);
  cout << "removed empty clusters" << endl; 
  v.print();
}

int main(int argc, char** argv) {
  cout << endl << "Hello World!" << endl;

  // load some data
  matrixD data;
  LoadData("SynData2.csv", data);
  int num_cols = data.size2();
  int num_rows = data.size1();
  //
  map<int, vector<double> > data_map;
  cout << "populating data_map" << endl;
  for(int row_idx=0; row_idx<num_rows; row_idx++) {
    data_map[row_idx] = extract_row(data, row_idx);
  }

  // create the view
  vector<int> global_column_indices;
  for(int col_idx=0; col_idx<data.size2(); col_idx++) {
    global_column_indices.push_back(col_idx);
  }
  View v = View(data, global_column_indices, 31);

  // print the initial view
  print_with_header(v, "empty view print");


  cout << "insert a single row:";
  int row_idx = 0;
  vector<double> row = extract_row(data, row_idx);
  v.insert_row(row, row_idx);
  print_with_header(v, "view after inserting single row");

  cout << "remove a single row:";
  row_idx = 0;
  row = extract_row(data, row_idx);
  v.remove_row(row, row_idx);
  print_with_header(v, "view after removeing single row");
  
  // populate the objects to test
  cout << endl << "populating objects" << endl;
  cout << "=================================" << endl;
  cout << "Insertings row:";
  for(int row_idx=0; row_idx<num_rows; row_idx++) {
    cout << " " << row_idx;
    vector<double> row = extract_row(data, row_idx);
    v.insert_row(row, row_idx);
  }
  print_with_header(v, "view after population");

  cout << "====================" << endl;
  cout << "Sampling" << endl;
  // test transition
  RandomNumberGenerator rng = RandomNumberGenerator();
  for(int iter=0; iter<21; iter++) {
    v.assert_state_consistency();
    v.transition_zs(data_map);
    v.transition_crp_alpha();
    v.transition_hypers();
    // if(iter % 10 == 0) {
    if(iter % 1 == 0) {
      cout << "Done iter: " << iter << endl;
      print_with_header(v, "view after iter");
    }
  }
  cout << "Done transition_zs" << endl;
  cout << endl;

  cout << "=====================" << endl;
  cout << "=====================" << endl;
  cout << "=====================" << endl;
  int remove_col_idx = 2;
  cout << "removing column: " << remove_col_idx;
  v.remove_col(remove_col_idx);
  print_with_header(v, "view after column removal");
  
  // empty object and verify empty
  remove_all_data(v, data_map);
  v.print();

  cout << "insert a single row: " << flush;
  row_idx = 0;
  row = extract_row(data, row_idx);

  vector<int> global_indices(row.size(),1);
  global_indices[0] = 0;
  std::partial_sum(global_indices.begin(), global_indices.end(), global_indices.begin());
  vector<double> aligned_row = v.align_data(row, global_indices);

  cout << aligned_row << endl << flush;
  v.insert_row(aligned_row, row_idx);
  print_with_header(v, "view after inserting single row");

  cout << "remove a single row:";
  v.remove_row(aligned_row, row_idx);
  print_with_header(v, "view after removeing single row");

  cout << endl << "Goodbye World!" << endl;
}