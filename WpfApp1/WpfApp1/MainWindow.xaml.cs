using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace WpfApp1
{
    // ListView 와 ListViewItem 을 상속, 윈도우 탐색기와 비슷한 동작 구현
    // https://stackoverflow.com/questions/1553622/wpf-drag-drop-from-listbox-with-selectionmode-multiple
    class ListViewItemEx : ListViewItem
    {
        private bool _deferSelection = false;

        public ListViewItemEx()
        {
            this.MouseMove += OnMouseMove;
            AllowDrop = true;
        }

        protected override void OnMouseLeftButtonDown(MouseButtonEventArgs e)
        {
            if (e.ClickCount == 1 && IsSelected)
            {
                // the user may start a drag by clicking into selected items
                // delay destroying the selection to the Up event
                _deferSelection = true;
            }
            else
            {
                base.OnMouseLeftButtonDown(e);
            }
        }

        protected override void OnMouseLeftButtonUp(MouseButtonEventArgs e)
        {
            if (_deferSelection)
            {
                try
                {
                    base.OnMouseLeftButtonDown(e);
                }
                finally
                {
                    _deferSelection = false;
                }
            }
            base.OnMouseLeftButtonUp(e);
        }

        protected override void OnMouseLeave(MouseEventArgs e)
        {
            // abort deferred Down
            _deferSelection = false;
            base.OnMouseLeave(e);
        }

        void OnMouseMove(object sender, MouseEventArgs e)
        {
            var item = sender as ListViewItemEx;
            if (item == null)
                return;

            var view = ItemsControl.ItemsControlFromItemContainer(item) as ListViewEx;
            if (view == null)
                return;
            
            // 컨텐츠가 string 이므로..
            string s = item.Content as string;
            bool contains = view.SelectedItems.Contains(s);
            bool pressed = e.LeftButton == MouseButtonState.Pressed;
            if (contains && pressed)
            {
                DragDrop.DoDragDrop(
                    item, 
                    new DataObject(typeof(System.Collections.IList), view.SelectedItems), DragDropEffects.Copy);
            }
        }

        protected override void OnDrop(DragEventArgs e)
        {
            // selectedItems 를 획득
            var list = e.Data.GetData(typeof(System.Collections.IList)) as System.Collections.IList;
            
            // 표시용
            string target = this.Content as string;
            string sources = "";
            foreach (string elem in list)
            {
                sources += elem + ", ";
            }
            char[] d = {' ', ','};
            sources = sources.TrimEnd(d);
            string msg = "sources : " + sources +"\ntarget : " + target;
            MessageBox.Show(msg);
            base.OnDrop(e);
        }
    }

    public class ListViewEx : ListView
    {
        protected override DependencyObject GetContainerForItemOverride()
        {
            return new ListViewItemEx();
        }
    }

    /// <summary>
    /// MainWindow.xaml에 대한 상호 작용 논리
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            for (int i =0; i< 10; ++i)
            {
                listView.Items.Add("hello" + Convert.ToString(i));
            }
        }
    }
}
